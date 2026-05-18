"""
Double McHarvard with Cheese — Pipeline Simulator GUI
Restaurant-themed Tkinter UI for CSEN 601 CA Project (Package 4)
"""

import tkinter as tk
from tkinter import ttk, scrolledtext, filedialog, font
import subprocess
import os
import sys
import re
import threading
import time

# ── Paths ─────────────────────────────────────────────────────────────────────
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# ── Color palette (restaurant/diner theme) ────────────────────────────────────
BG_MAIN      = "#1a0a00"   # dark espresso
BG_SIDEBAR   = "#2b1500"   # dark mahogany
BG_RECEIPT   = "#fffde7"   # parchment / receipt paper
BG_HEADER    = "#c0392b"   # ketchup red
BG_STRIPE    = "#fff9e6"   # cream stripe
BG_GOLD      = "#f39c12"   # cheese gold
FG_MAIN      = "#fff8f0"   # warm white
FG_DARK      = "#1a0a00"   # dark text on light bg
FG_ACCENT    = "#f39c12"   # golden accent
FG_STAGE_IF  = "#e67e22"   # orange – IF
FG_STAGE_ID  = "#27ae60"   # green  – ID
FG_STAGE_EX  = "#c0392b"   # red    – EX
FG_STAGE_FWD = "#8e44ad"   # purple – FWD
FG_STAGE_END = "#2980b9"   # blue   – end/latch
FG_STALL     = "#e74c3c"   # bright red – stall/flush
FG_CHEESE    = "#f1c40f"   # bright yellow for Package 4 cheese effect

RECEIPT_FONT   = ("Courier New", 10)
RECEIPT_FONT_B = ("Courier New", 10, "bold")
HEADER_FONT    = ("Courier New", 13, "bold")
TITLE_FONT     = ("Impact", 20, "bold")
CHEESE_FONT    = ("Comic Sans MS", 12, "bold")

RECEIPT_WIDTH  = 52   # characters wide for receipt formatting

# ── Opcode name map ────────────────────────────────────────────────────────────
OPCODE_NAMES = {
    "0": "ADD", "1": "SUB", "2": "MUL", "3": "LDI",
    "4": "BEQZ","5": "AND", "6": "OR",  "7": "JR",
    "8": "SAL", "9": "SAR","10": "LB", "11": "SB",
}

STAGE_ICONS = {
    "IF":  "🍔",
    "ID":  "🧅",
    "EX":  "🔥",
    "FWD": "🧀",
    "END": "🥡",
    "FIN": "🏁",
}

# ══════════════════════════════════════════════════════════════════════════════
class SimulatorGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("🍔 Double McHarvard with Cheese — Pipeline Simulator")
        self.root.configure(bg=BG_MAIN)
        self.root.geometry("1300x800")
        self.root.minsize(1000, 650)

        # State
        self.sim_output_lines: list[str] = []
        self.cycles: list[list[str]] = []       # cycles[i] = lines for cycle i+1
        self.final_lines: list[str] = []        # everything after last cycle
        self.current_cycle_idx = -1             # which cycle we're viewing
        self.all_cycle_receipts: list[str] = [] # full receipt text per cycle (for sidebar)
        self.sim_path = ""
        self.program_path = ""
        self.simulation_done = False

        self._build_ui()

    # ── UI construction ────────────────────────────────────────────────────────
    def _build_ui(self):
        self._build_menu_bar()
        self._build_header()

        # Main content area
        content = tk.Frame(self.root, bg=BG_MAIN)
        content.pack(fill=tk.BOTH, expand=True, padx=8, pady=(0, 8))

        # Left: controls + current receipt
        left = tk.Frame(content, bg=BG_MAIN)
        left.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self._build_controls(left)
        self._build_receipt_area(left)

        # Right: cycle history sidebar
        self._build_sidebar(content)

        self._build_status_bar()

    def _build_menu_bar(self):
        menubar = tk.Menu(self.root, bg=BG_HEADER, fg=FG_MAIN,
                          activebackground=BG_GOLD, activeforeground=FG_DARK)
        self.root.config(menu=menubar)

        file_menu = tk.Menu(menubar, tearoff=0, bg=BG_SIDEBAR, fg=FG_MAIN)
        menubar.add_cascade(label="📁 File", menu=file_menu)
        file_menu.add_command(label="Open Program File…", command=self._browse_program)
        file_menu.add_command(label="Select Simulator Executable…", command=self._browse_sim)
        file_menu.add_separator()
        file_menu.add_command(label="Quit", command=self.root.quit)

        help_menu = tk.Menu(menubar, tearoff=0, bg=BG_SIDEBAR, fg=FG_MAIN)
        menubar.add_cascade(label="❓ Help", menu=help_menu)
        help_menu.add_command(label="About", command=self._show_about)

    def _build_header(self):
        hdr = tk.Frame(self.root, bg=BG_HEADER, pady=6)
        hdr.pack(fill=tk.X, padx=0, pady=0)

        tk.Label(hdr, text="🍔", font=("", 28), bg=BG_HEADER).pack(side=tk.LEFT, padx=12)

        title_frame = tk.Frame(hdr, bg=BG_HEADER)
        title_frame.pack(side=tk.LEFT)

        tk.Label(title_frame, text="DOUBLE McHARVARD",
                 font=TITLE_FONT, bg=BG_HEADER, fg=FG_MAIN).pack(anchor="w")

        # Cheesy effect for Package 4 subtitle
        cheese_label = tk.Label(title_frame,
                                text="✨ W I T H   C H E E S E ✨   — Package 4 Pipeline Simulator",
                                font=CHEESE_FONT, bg=BG_HEADER, fg=FG_CHEESE)
        cheese_label.pack(anchor="w")
        self._animate_cheese(cheese_label)

        tk.Label(hdr, text="🍟", font=("", 28), bg=BG_HEADER).pack(side=tk.RIGHT, padx=12)

        # Sub-header strip
        sub = tk.Frame(self.root, bg=BG_GOLD, pady=2)
        sub.pack(fill=tk.X)
        tk.Label(sub, text="CSEN 601  •  Harvard Architecture  •  Circular Shifts  •  Serving fresh pipeline cycles since 2026",
                 font=("Courier New", 9, "bold"), bg=BG_GOLD, fg=FG_DARK).pack()

    def _animate_cheese(self, label):
        """Wavy cheese colour animation."""
        colors = [FG_CHEESE, "#f39c12", "#e67e22", "#f1c40f", "#fff176", FG_CHEESE]
        self._cheese_colors = colors
        self._cheese_idx = 0

        def _cycle():
            label.config(fg=self._cheese_colors[self._cheese_idx % len(colors)])
            self._cheese_idx += 1
            self.root.after(300, _cycle)
        _cycle()

    def _build_controls(self, parent):
        ctrl = tk.LabelFrame(parent, text=" 🧑‍🍳  Kitchen Controls ",
                              bg=BG_SIDEBAR, fg=FG_ACCENT,
                              font=RECEIPT_FONT_B, bd=2, relief=tk.RIDGE)
        ctrl.pack(fill=tk.X, padx=4, pady=4)

        row1 = tk.Frame(ctrl, bg=BG_SIDEBAR)
        row1.pack(fill=tk.X, padx=6, pady=4)

        tk.Label(row1, text="Program (.txt):", bg=BG_SIDEBAR, fg=FG_MAIN,
                 font=RECEIPT_FONT).pack(side=tk.LEFT)
        self.program_var = tk.StringVar(value="(none selected)")
        tk.Label(row1, textvariable=self.program_var, bg=BG_SIDEBAR, fg=FG_ACCENT,
                 font=RECEIPT_FONT, width=30, anchor="w").pack(side=tk.LEFT, padx=4)
        tk.Button(row1, text="Browse…", command=self._browse_program,
                  bg=BG_GOLD, fg=FG_DARK, font=RECEIPT_FONT_B,
                  relief=tk.FLAT, cursor="hand2").pack(side=tk.LEFT)

        row2 = tk.Frame(ctrl, bg=BG_SIDEBAR)
        row2.pack(fill=tk.X, padx=6, pady=2)

        tk.Label(row2, text="Simulator (.exe):", bg=BG_SIDEBAR, fg=FG_MAIN,
                 font=RECEIPT_FONT).pack(side=tk.LEFT)
        self.sim_var = tk.StringVar(value="(none selected)")
        tk.Label(row2, textvariable=self.sim_var, bg=BG_SIDEBAR, fg=FG_ACCENT,
                 font=RECEIPT_FONT, width=30, anchor="w").pack(side=tk.LEFT, padx=4)
        tk.Button(row2, text="Browse…", command=self._browse_sim,
                  bg=BG_GOLD, fg=FG_DARK, font=RECEIPT_FONT_B,
                  relief=tk.FLAT, cursor="hand2").pack(side=tk.LEFT)

        row3 = tk.Frame(ctrl, bg=BG_SIDEBAR)
        row3.pack(fill=tk.X, padx=6, pady=6)

        self.run_btn = tk.Button(row3, text="🔥  FIRE UP THE GRILL  (Run Simulation)",
                                 command=self._run_simulation,
                                 bg=BG_HEADER, fg=FG_MAIN, font=("Courier New", 11, "bold"),
                                 relief=tk.FLAT, cursor="hand2", pady=4)
        self.run_btn.pack(side=tk.LEFT, fill=tk.X, expand=True)

        self.next_btn = tk.Button(row3,
                                  text="Next Cycle ▶  [Enter]",
                                  command=self._next_cycle,
                                  bg="#27ae60", fg=FG_MAIN, font=RECEIPT_FONT_B,
                                  relief=tk.FLAT, cursor="hand2", pady=4,
                                  state=tk.DISABLED)
        self.next_btn.pack(side=tk.LEFT, padx=(8, 0))

        self.prev_btn = tk.Button(row3,
                                  text="◀ Prev",
                                  command=self._prev_cycle,
                                  bg="#2980b9", fg=FG_MAIN, font=RECEIPT_FONT_B,
                                  relief=tk.FLAT, cursor="hand2", pady=4,
                                  state=tk.DISABLED)
        self.prev_btn.pack(side=tk.LEFT, padx=(8, 0))

        # Bind Enter key globally
        self.root.bind("<Return>", lambda e: self._next_cycle())
        self.root.bind("<KP_Enter>", lambda e: self._next_cycle())

    def _build_receipt_area(self, parent):
        lf = tk.LabelFrame(parent,
                            text=" 🧾  Current Order Receipt ",
                            bg=BG_SIDEBAR, fg=FG_ACCENT,
                            font=RECEIPT_FONT_B, bd=2, relief=tk.RIDGE)
        lf.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

        self.receipt_text = tk.Text(lf, bg=BG_RECEIPT, fg=FG_DARK,
                                    font=RECEIPT_FONT, relief=tk.FLAT,
                                    insertbackground=FG_DARK, state=tk.DISABLED,
                                    wrap=tk.WORD, selectbackground=BG_GOLD)
        scroll = tk.Scrollbar(lf, command=self.receipt_text.yview, bg=BG_SIDEBAR)
        self.receipt_text.configure(yscrollcommand=scroll.set)
        scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self.receipt_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # Define tags for coloring
        self.receipt_text.tag_config("header",    font=HEADER_FONT,    foreground=BG_HEADER)
        self.receipt_text.tag_config("stage_IF",  font=RECEIPT_FONT_B, foreground=FG_STAGE_IF)
        self.receipt_text.tag_config("stage_ID",  font=RECEIPT_FONT_B, foreground=FG_STAGE_ID)
        self.receipt_text.tag_config("stage_EX",  font=RECEIPT_FONT_B, foreground=FG_STAGE_EX)
        self.receipt_text.tag_config("stage_FWD", font=RECEIPT_FONT_B, foreground=FG_STAGE_FWD)
        self.receipt_text.tag_config("stage_END", font=RECEIPT_FONT_B, foreground=FG_STAGE_END)
        self.receipt_text.tag_config("stall",     font=RECEIPT_FONT_B, foreground=FG_STALL)
        self.receipt_text.tag_config("gold",      font=RECEIPT_FONT_B, foreground=BG_GOLD)
        self.receipt_text.tag_config("normal",    font=RECEIPT_FONT,   foreground=FG_DARK)
        self.receipt_text.tag_config("separator", font=RECEIPT_FONT,   foreground="#999999")
        self.receipt_text.tag_config("cheese",    font=CHEESE_FONT,    foreground="#c0392b")
        self.receipt_text.tag_config("final_hdr", font=HEADER_FONT,    foreground="#2980b9")

        self._show_welcome_receipt()

    def _build_sidebar(self, parent):
        sidebar_frame = tk.Frame(parent, bg=BG_SIDEBAR, width=300)
        sidebar_frame.pack(side=tk.RIGHT, fill=tk.Y, padx=(4, 4), pady=0)
        sidebar_frame.pack_propagate(False)

        hdr = tk.Label(sidebar_frame,
                       text="📋  Cycle History\n(All Past Receipts)",
                       bg=BG_HEADER, fg=FG_MAIN, font=RECEIPT_FONT_B,
                       pady=6)
        hdr.pack(fill=tk.X)

        # Show History button
        self.history_btn = tk.Button(sidebar_frame,
                                     text="🗂  Show Full History",
                                     command=self._show_history_window,
                                     bg=BG_GOLD, fg=FG_DARK,
                                     font=RECEIPT_FONT_B, relief=tk.FLAT,
                                     cursor="hand2", pady=4)
        self.history_btn.pack(fill=tk.X, padx=4, pady=4)

        # Mini receipt list
        self.history_list = tk.Listbox(sidebar_frame,
                                        bg=BG_RECEIPT, fg=FG_DARK,
                                        font=("Courier New", 9),
                                        selectbackground=BG_GOLD,
                                        selectforeground=FG_DARK,
                                        relief=tk.FLAT,
                                        activestyle="dotbox")
        hist_scroll = tk.Scrollbar(sidebar_frame, command=self.history_list.yview, bg=BG_SIDEBAR)
        self.history_list.configure(yscrollcommand=hist_scroll.set)
        hist_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self.history_list.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(4, 0))
        self.history_list.bind("<<ListboxSelect>>", self._on_history_click)

    def _build_status_bar(self):
        self.status_var = tk.StringVar(value="🍔 Welcome! Select your simulator and program file to begin.")
        status = tk.Label(self.root, textvariable=self.status_var,
                          bg=BG_GOLD, fg=FG_DARK,
                          font=("Courier New", 9, "bold"),
                          anchor="w", padx=8, pady=2)
        status.pack(fill=tk.X, side=tk.BOTTOM)

    # ── Receipt rendering ──────────────────────────────────────────────────────
    def _show_welcome_receipt(self):
        self._clear_receipt()
        lines = [
            ("═" * RECEIPT_WIDTH, "separator"),
            ("    🍔  DOUBLE McHARVARD  🍔    ", "header"),
            ("       WITH CHEESE CIRCULAR SHIFTS   ", "cheese"),
            ("═" * RECEIPT_WIDTH, "separator"),
            ("", "normal"),
            ("  Welcome to the Pipeline Diner!", "normal"),
            ("  Your order will be served cycle", "normal"),
            ("  by cycle as a fresh receipt. 🧾", "normal"),
            ("", "normal"),
            ("  HOW TO ORDER:", "gold"),
            ("  1. Browse to your simulator .exe", "normal"),
            ("  2. Browse to your program .txt", "normal"),
            ("  3. Click FIRE UP THE GRILL 🔥", "normal"),
            ("  4. Press [Enter] or Next Cycle ▶", "normal"),
            ("     to advance through cycles", "normal"),
            ("", "normal"),
            ("  Press Enter on the keypad to", "normal"),
            ("  advance to the next clock cycle!", "normal"),
            ("", "normal"),
            ("─" * RECEIPT_WIDTH, "separator"),
            ("  Package 4: Double McHarvard   ", "cheese"),
            ("           with Cheese 🧀        ", "cheese"),
            ("─" * RECEIPT_WIDTH, "separator"),
        ]
        self._write_receipt(lines)

    def _clear_receipt(self):
        self.receipt_text.config(state=tk.NORMAL)
        self.receipt_text.delete("1.0", tk.END)
        self.receipt_text.config(state=tk.DISABLED)

    def _write_receipt(self, lines):
        """lines = list of (text, tag) tuples"""
        self.receipt_text.config(state=tk.NORMAL)
        self.receipt_text.delete("1.0", tk.END)
        for text, tag in lines:
            self.receipt_text.insert(tk.END, text + "\n", tag)
        self.receipt_text.config(state=tk.DISABLED)
        self.receipt_text.see("1.0")

    def _render_cycle_receipt(self, cycle_idx: int):
        """Build and display the receipt for cycle_idx (0-based)."""
        if cycle_idx < 0 or cycle_idx >= len(self.cycles):
            return
        lines_raw = self.cycles[cycle_idx]
        cycle_num = cycle_idx + 1
        receipt = self._build_receipt_lines(cycle_num, lines_raw)
        self._write_receipt(receipt)

    def _build_receipt_lines(self, cycle_num: int, raw_lines: list[str]) -> list:
        """Convert raw simulator output lines into styled receipt tuples."""
        W = RECEIPT_WIDTH
        sep = ("─" * W, "separator")
        thick = ("═" * W, "separator")

        out = []
        out.append(thick)
        out.append(("  🍔  McHARVARD PIPELINE DINER  🍔  ", "header"))
        out.append(("  Harvard Architecture — Package 4  ", "header"))
        out.append(thick)
        out.append((f"  CLOCK CYCLE:  #{cycle_num:<6}   TABLE FOR 1   ", "gold"))
        out.append(("  ─── PIPELINE STAGES THIS CYCLE ───", "separator"))
        out.append(sep)

        for raw in raw_lines:
            line = raw.strip()
            if not line:
                continue

            # Skip the header line "====" since we already rendered it
            if line.startswith("====="):
                continue

            styled = self._classify_line(line, cycle_num)
            out.append(styled)

        out.append(sep)
        out.append(("  🧾  Order complete — press Enter  ", "normal"))
        out.append(("      for the next course!         ", "normal"))
        out.append(thick)

        return out

    def _classify_line(self, line: str, cycle_num: int):
        """Return (display_text, tag) for a raw simulator output line."""
        icon = ""
        tag = "normal"

        if re.match(r"\[IF\s*\|", line):
            icon = STAGE_ICONS["IF"]
            tag = "stage_IF"
            line = re.sub(r"\[IF\s*\|\s*cycle \d+\]\s*", "", line)
            line = f"  {icon} FETCH        {line}"
        elif re.match(r"\[ID\s*\|", line):
            icon = STAGE_ICONS["ID"]
            tag = "stage_ID"
            line = re.sub(r"\[ID\s*\|\s*cycle \d+\]\s*", "", line)
            line = f"  {icon} DECODE       {line}"
        elif re.match(r"\[EX\s*\|", line):
            icon = STAGE_ICONS["EX"]
            tag = "stage_EX"
            line = re.sub(r"\[EX\s*\|\s*cycle \d+\]\s*", "", line)
            line = f"  {icon} EXECUTE      {line}"
        elif re.match(r"\[FWD\s*\|", line):
            icon = STAGE_ICONS["FWD"]
            tag = "stage_FWD"
            line = re.sub(r"\[FWD\s*\|\s*cycle \d+\]\s*", "", line)
            line = f"  {icon} FORWARDING   {line}"
        elif re.match(r"\[LATCH", line) or re.match(r"---\s*End of", line):
            icon = STAGE_ICONS["END"]
            tag = "stage_END"
            line = f"  {icon} LATCH        {line}"
        elif "STALL" in line.upper():
            tag = "stall"
            line = f"  ⚠️  {line}"
        elif "flushed" in line.lower():
            tag = "stall"
            line = f"  🚫 {line}"
        elif line.startswith("R") and "changed to" in line:
            tag = "gold"
            line = f"  📝 REG UPDATE  {line}"
        elif line.startswith("SREG"):
            tag = "gold"
            line = f"  🔢 FLAGS       {line}"
        elif "dataMemory" in line:
            tag = "stage_FWD"
            line = f"  💾 MEM STORE   {line}"
        elif "no more instructions" in line:
            tag = "separator"
            line = f"  🏁 {line}"
        else:
            tag = "normal"
            line = f"  {line}"

        return (line, tag)

    def _render_final_receipt(self):
        """Render the final state receipt."""
        W = RECEIPT_WIDTH
        out = []
        out.append(("═" * W, "separator"))
        out.append(("  🏁  FINAL STATE — MEAL COMPLETE!  ", "final_hdr"))
        out.append(("  Double McHarvard with Cheese 🧀   ", "cheese"))
        out.append(("═" * W, "separator"))

        section = "other"
        for line in self.final_lines:
            line = line.strip()
            if not line:
                out.append(("", "normal"))
                continue
            if "Registers:" in line:
                section = "registers"
                out.append(("  📋  REGISTER FILE:", "gold"))
                continue
            elif "Instruction Memory" in line:
                section = "imem"
                out.append(("  🗂  " + line, "stage_IF"))
                continue
            elif "Data Memory" in line:
                section = "dmem"
                out.append(("  💾  " + line, "stage_FWD"))
                continue
            elif "Final State" in line or "=====" in line:
                continue

            if section == "registers":
                out.append(("    " + line, "stage_ID"))
            elif section == "imem":
                out.append(("    " + line, "stage_IF"))
            elif section == "dmem":
                out.append(("    " + line, "stage_FWD"))
            else:
                out.append(("  " + line, "normal"))

        out.append(("─" * W, "separator"))
        out.append(("  Thank you for dining at McHarvard! ", "cheese"))
        out.append(("  Come back for more pipeline cycles! ", "normal"))
        out.append(("═" * W, "separator"))

        self._write_receipt(out)

    # ── Simulation running ─────────────────────────────────────────────────────
    def _browse_program(self):
        path = filedialog.askopenfilename(
            title="Select Program File",
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")])
        if path:
            self.program_path = path
            self.program_var.set(os.path.basename(path))
            self._set_status(f"📄 Program loaded: {os.path.basename(path)}")

    def _browse_sim(self):
        path = filedialog.askopenfilename(
            title="Select Simulator Executable",
            filetypes=[("Executables", "*.exe *.out sim simulator"), ("All files", "*.*")])
        if path:
            self.sim_path = path
            self.sim_var.set(os.path.basename(path))
            self._set_status(f"⚙️  Simulator loaded: {os.path.basename(path)}")

    def _run_simulation(self):
        if not self.sim_path:
            self._set_status("❌ Please select a simulator executable first!")
            return
        if not self.program_path:
            self._set_status("❌ Please select a program file first!")
            return

        self._set_status("🔥 Firing up the grill… running simulation…")
        self.run_btn.config(state=tk.DISABLED, text="🔥 Cooking…")
        self.next_btn.config(state=tk.DISABLED)
        self.prev_btn.config(state=tk.DISABLED)

        # Reset state
        self.sim_output_lines = []
        self.cycles = []
        self.final_lines = []
        self.all_cycle_receipts = []
        self.current_cycle_idx = -1
        self.simulation_done = False
        self.history_list.delete(0, tk.END)

        threading.Thread(target=self._run_sim_thread, daemon=True).start()

    def _run_sim_thread(self):
        """Run simulator in a thread, copy program file next to exe, parse output."""
        import shutil, tempfile

        sim_dir = os.path.dirname(self.sim_path)
        sim_name = os.path.basename(self.sim_path)

        # Copy program as program.txt in sim dir (that's what main.c reads)
        test_path = os.path.join(sim_dir, "program.txt")
        try:
            shutil.copy2(self.program_path, test_path)
        except Exception as e:
            self.root.after(0, self._sim_error, f"Could not copy program file: {e}")
            return

        try:
            result = subprocess.run(
                [self.sim_path],
                cwd=sim_dir,
                capture_output=True,
                text=True,
                timeout=30,
                encoding="utf-8",
                errors="replace"
            )
            raw = result.stdout + result.stderr
        except Exception as e:
            self.root.after(0, self._sim_error, f"Simulation failed: {e}")
            return

        self.sim_output_lines = raw.splitlines()
        self.root.after(0, self._parse_and_start)

    def _sim_error(self, msg):
        self._set_status(f"❌ {msg}")
        self.run_btn.config(state=tk.NORMAL, text="🔥  FIRE UP THE GRILL  (Run Simulation)")

    def _parse_and_start(self):
        """Split raw output into per-cycle chunks and final state."""
        lines = self.sim_output_lines
        current_block: list[str] = []
        in_final = False

        for line in lines:
            if re.match(r"={5,}\s*Clock Cycle \d+", line):
                if current_block:
                    self.cycles.append(current_block)
                current_block = [line]
            elif re.match(r"={5,}\s*Final State", line):
                if current_block:
                    self.cycles.append(current_block)
                current_block = []
                in_final = True
                self.final_lines.append(line)
            elif in_final:
                self.final_lines.append(line)
            else:
                current_block.append(line)

        if current_block and not in_final:
            self.cycles.append(current_block)

        if not self.cycles:
            self._set_status("⚠️  No cycle output found. Check your program file and simulator.")
            self.run_btn.config(state=tk.NORMAL, text="🔥  FIRE UP THE GRILL  (Run Simulation)")
            return

        total = len(self.cycles)
        self._set_status(f"✅ Simulation complete! {total} clock cycles ready. Press Enter or Next ▶ to begin!")
        self.run_btn.config(state=tk.NORMAL, text="🔥  FIRE UP THE GRILL  (Run Simulation)")
        self.next_btn.config(state=tk.NORMAL)
        self.simulation_done = True
        self.current_cycle_idx = -1

        # Show a "ready" receipt
        self._show_ready_receipt(total)

    def _show_ready_receipt(self, total):
        W = RECEIPT_WIDTH
        lines = [
            ("═" * W, "separator"),
            ("  🍔  ORDER READY TO SERVE!  🍔   ", "header"),
            ("═" * W, "separator"),
            (f"  Total Clock Cycles:  {total}", "gold"),
            ("  Press [Enter] or Next ▶         ", "normal"),
            ("  to serve cycle #1               ", "normal"),
            ("─" * W, "separator"),
            ("  McHarvard Harvard Architecture  ", "cheese"),
            ("  Pipelined  •  With Cheese 🧀    ", "cheese"),
            ("═" * W, "separator"),
        ]
        self._write_receipt(lines)

    def _next_cycle(self):
        if not self.simulation_done:
            return

        next_idx = self.current_cycle_idx + 1
        total = len(self.cycles)

        if next_idx >= total:
            # Show final state
            self.current_cycle_idx = total  # mark as past the last cycle
            self._render_final_receipt()
            self._set_status("🏁 Final state displayed! Use Prev ◀ to review cycles.")
            self.next_btn.config(state=tk.DISABLED)
            self.prev_btn.config(state=tk.NORMAL)
            return

        self.current_cycle_idx = next_idx
        self._render_cycle_receipt(self.current_cycle_idx)
        self._add_to_history(self.current_cycle_idx)
        self._set_status(f"🍔 Serving Clock Cycle #{next_idx + 1} of {total} — bon appétit!")

        self.prev_btn.config(state=tk.NORMAL)
        if next_idx + 1 >= total:
            self.next_btn.config(text="Final State ▶▶  [Enter]")
        else:
            self.next_btn.config(text="Next Cycle ▶  [Enter]")

    def _prev_cycle(self):
        if self.current_cycle_idx <= 0:
            return
        self.current_cycle_idx -= 1
        self._render_cycle_receipt(self.current_cycle_idx)
        total = len(self.cycles)
        self._set_status(f"◀ Viewing Clock Cycle #{self.current_cycle_idx + 1} of {total}")
        self.next_btn.config(state=tk.NORMAL,
                             text="Next Cycle ▶  [Enter]" if self.current_cycle_idx + 1 < total
                             else "Final State ▶▶  [Enter]")
        if self.current_cycle_idx == 0:
            self.prev_btn.config(state=tk.DISABLED)

    def _add_to_history(self, cycle_idx: int):
        """Add a summary entry to the sidebar list."""
        raw = self.cycles[cycle_idx]
        cycle_num = cycle_idx + 1

        # Find what executed
        exec_line = next((l for l in raw if re.search(r"\[EX\s*\|", l)), "")
        short = re.sub(r"\[EX\s*\|\s*cycle \d+\]\s*", "", exec_line).strip()
        if not short:
            short = "(bubble)"

        entry = f"  #{cycle_num:>3}  {short[:22]}"
        self.history_list.insert(tk.END, entry)
        self.history_list.see(tk.END)

    def _on_history_click(self, event):
        sel = self.history_list.curselection()
        if not sel:
            return
        idx = sel[0]
        self.current_cycle_idx = idx
        self._render_cycle_receipt(idx)
        total = len(self.cycles)
        self._set_status(f"📋 Viewing history: Clock Cycle #{idx + 1} of {total}")
        self.prev_btn.config(state=tk.NORMAL if idx > 0 else tk.DISABLED)
        self.next_btn.config(state=tk.NORMAL)

    # ── History popup window ───────────────────────────────────────────────────
    def _show_history_window(self):
        if not self.simulation_done or not self.cycles:
            self._set_status("⚠️  Run a simulation first to see cycle history.")
            return

        win = tk.Toplevel(self.root)
        win.title("📋  Full Cycle History — All Receipts")
        win.configure(bg=BG_MAIN)
        win.geometry("700x600")

        hdr = tk.Label(win, text="🧾  ALL CYCLE RECEIPTS — McHARVARD DINER",
                       bg=BG_HEADER, fg=FG_MAIN, font=HEADER_FONT, pady=8)
        hdr.pack(fill=tk.X)

        frame = tk.Frame(win, bg=BG_MAIN)
        frame.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

        txt = scrolledtext.ScrolledText(frame, bg=BG_RECEIPT, fg=FG_DARK,
                                         font=("Courier New", 9), relief=tk.FLAT,
                                         state=tk.NORMAL)
        txt.pack(fill=tk.BOTH, expand=True)

        # Tag styles
        txt.tag_config("header",    font=("Courier New", 10, "bold"), foreground=BG_HEADER)
        txt.tag_config("stage_IF",  font=("Courier New", 9, "bold"),  foreground=FG_STAGE_IF)
        txt.tag_config("stage_ID",  font=("Courier New", 9, "bold"),  foreground=FG_STAGE_ID)
        txt.tag_config("stage_EX",  font=("Courier New", 9, "bold"),  foreground=FG_STAGE_EX)
        txt.tag_config("stage_FWD", font=("Courier New", 9, "bold"),  foreground=FG_STAGE_FWD)
        txt.tag_config("gold",      font=("Courier New", 9, "bold"),  foreground=BG_GOLD)
        txt.tag_config("separator", font=("Courier New", 9),          foreground="#888888")
        txt.tag_config("cheese",    font=("Courier New", 9, "bold"),  foreground="#c0392b")
        txt.tag_config("normal",    font=("Courier New", 9),          foreground=FG_DARK)
        txt.tag_config("stall",     font=("Courier New", 9, "bold"),  foreground=FG_STALL)
        txt.tag_config("stage_END", font=("Courier New", 9, "bold"),  foreground=FG_STAGE_END)
        txt.tag_config("final_hdr", font=("Courier New", 9, "bold"),  foreground="#2980b9")

        txt.insert(tk.END, "═" * 60 + "\n", "separator")
        txt.insert(tk.END, "   🍔  McHARVARD PIPELINE DINER — FULL ORDER HISTORY  🍔\n", "header")
        txt.insert(tk.END, "═" * 60 + "\n\n", "separator")

        for i, cycle_raw in enumerate(self.cycles):
            receipt_lines = self._build_receipt_lines(i + 1, cycle_raw)
            for text, tag in receipt_lines:
                txt.insert(tk.END, text + "\n", tag)
            txt.insert(tk.END, "\n")

        if self.final_lines:
            final_lines = self._render_final_receipt_lines()
            for text, tag in final_lines:
                txt.insert(tk.END, text + "\n", tag)

        txt.config(state=tk.DISABLED)

        tk.Button(win, text="Close", command=win.destroy,
                  bg=BG_HEADER, fg=FG_MAIN, font=RECEIPT_FONT_B,
                  relief=tk.FLAT, cursor="hand2").pack(pady=6)

    def _render_final_receipt_lines(self) -> list:
        W = 58
        out = []
        out.append(("═" * W, "separator"))
        out.append(("  🏁  FINAL STATE — ALL DONE!", "final_hdr"))
        out.append(("═" * W, "separator"))
        section = "other"
        for line in self.final_lines:
            line = line.strip()
            if not line:
                out.append(("", "normal"))
                continue
            if "Registers:" in line:
                section = "registers"
                out.append(("  📋  REGISTER FILE:", "gold"))
                continue
            elif "Instruction Memory" in line:
                section = "imem"
                out.append(("  🗂  " + line, "stage_IF"))
                continue
            elif "Data Memory" in line:
                section = "dmem"
                out.append(("  💾  " + line, "stage_FWD"))
                continue
            elif "Final State" in line or "=====" in line:
                continue
            if section == "registers":
                out.append(("    " + line, "stage_ID"))
            elif section == "imem":
                out.append(("    " + line, "stage_IF"))
            elif section == "dmem":
                out.append(("    " + line, "stage_FWD"))
            else:
                out.append(("  " + line, "normal"))
        out.append(("═" * W, "separator"))
        return out

    # ── Misc ───────────────────────────────────────────────────────────────────
    def _set_status(self, msg: str):
        self.status_var.set(msg)

    def _show_about(self):
        win = tk.Toplevel(self.root)
        win.title("About")
        win.configure(bg=BG_MAIN)
        win.geometry("450x320")
        tk.Label(win, text="🍔 Double McHarvard with Cheese",
                 font=TITLE_FONT, bg=BG_MAIN, fg=FG_ACCENT).pack(pady=10)
        tk.Label(win, text="Pipeline Simulator GUI\nCSEN 601 — Package 4\n\nHarvard Architecture\nCircular Shifts\n\nBuilt with Python & Tkinter\n\n✨ WITH CHEESE ✨",
                 font=RECEIPT_FONT, bg=BG_MAIN, fg=FG_MAIN, justify=tk.CENTER).pack()
        tk.Button(win, text="Close", command=win.destroy,
                  bg=BG_HEADER, fg=FG_MAIN, font=RECEIPT_FONT_B,
                  relief=tk.FLAT).pack(pady=10)


# ══════════════════════════════════════════════════════════════════════════════
def main():
    root = tk.Tk()
    app = SimulatorGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()
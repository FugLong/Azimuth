#!/usr/bin/env python3
"""
Fill Description and Unit Price (USD) on Azimuth.kicad_sch *instance* symbols.
Prices are low-qty ballparks for BOM planning — refresh from LCSC/Mouser before ordering.
"""
from __future__ import annotations

import re
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
SCH = REPO / "kicad" / "Azimuth_Design" / "Azimuth.kicad_sch"

# LCSC C-code -> unit USD at the first listed quantity tier on product page
# (manual snapshot, 2026-04-07, no promo/discount column).
UNIT_PRICE_USD: dict[str, str] = {
    "C106224": "0.001",
    "C106225": "0.0011",
    "C128953": "0.1791",
    "C138135": "0.001",
    "C155743": "0.0011",
    "C172433": "0.0267",
    "C207022": "0.1119",
    "C20917": "0.0798",
    "C25767": "0.0007",
    "C274341": "0.0011",
    "C274872": "0.0011",
    "C274876": "0.0011",
    "C2765186": "0.0649",
    "C2934560": "3.0674",
    "C295747": "0.1947",
    "C295773": "0.0011",
    "C315248": "0.0087",
    "C327392": "0.0011",
    "C335106": "0.0212",
    "C347476": "0.0311",
    "C3646923": "0.069",
    "C485517": "0.1959",
    "C49654383": "0.0006",
    "C49655823": "0.001",
    "C49234124": "0.0186",
    "C5204746": "0.0193",
    "C5381776": "0.0145",
    "C60474": "0.001",
    "C64886": "0.0217",
    "C76934": "0.002",
    "C77623": "0.0011",
    "C779813": "0.0244",
    "C94598": "0.4246",
}

# BNO086: often sourced outside LCSC; rough Mouser-class 1-pc (refresh)
UNIT_PRICE_MPN: dict[str, str] = {
    "BNO086": "11.50",
}

# (reference, value) -> description (electrical / functional)
DESCS: dict[tuple[str, str], str] = {
    ("C4", "2.2uF"): "Bypass: LDO input / charger adjacent 2.2 µF ceramic (0201)",
    ("R4", "2K"): "TP4054 PROG: sets IBAT ≈ 500 mA (1000/R_kΩ mA)",
    ("R2", "220k"): "Battery divider low side: VBAT_SW tap to IO4 for ADC (with R1)",
    ("J1", "TYPE-C 16PIN 2MD(073)"): "USB-C receptacle: VBUS, GND, D+/D− (6-pin footprint)",
    ("R5", "150R"): "CHG1 current limit: red charge indicator from 3V3",
    ("C11", "100nf"): "BNO086 CAP pin bypass to GND (datasheet requirement)",
    ("CHG1", "XL-0201SURC"): "Charge-state LED (TP4054 CHRG) — 0201 red",
    ("U1", "ESP32-C3-WROOM-02-N4(4MB)"): "MCU + RF: Wi-Fi/BLE, USB-OTG PHY, 4 MB flash; Azimuth GPIO map per wiring.md",
    ("R11", "22R"): "USB D+ series resistor between module and connector",
    ("R18", "10k"): "Power control: TLV75733 EN / divider network (see schematic nets)",
    ("C2", "0.1uf"): "VBAT switched rail HF bypass at divider / switch node",
    ("C6", "2.2uF"): "TP4054 VCC bypass",
    ("R14", "10k"): "PS0/WAKE pull-up to 3V3 on IO2 (BNO086 SPI strap + ESP32 GPIO2)",
    ("R3", "100k"): "Power path bias / discharge (Q1 gate network per schematic)",
    ("D2", "B5819WS"): "Passive buzzer flyback Schottky: cathode to 3V3/+buzzer, anode to Q2 drain",
    ("Q1", "LP0404N3T5G"): "Power OR / ideal-diode style switch: VBAT vs VUSB path into LDO input",
    ("PH2.0", "S2B-PH-SM4-TB(LF)(SN)"): "1S LiPo JST-PH battery connector (horizontal SMD)",
    ("BAT_0", "DNP"): "Optional BAT+ test pad — do not populate by default",
    ("R21", "100K"): "Q2 NMOS gate discharge to GND (default buzzer off)",
    ("D1", "MBR0530_C5204746"): "Schottky (30 V / 0.5 A class): ORing / clamp in VBAT–LDO path with Q1 per schematic",
    ("C7", "2.2uF"): "TP4054 BAT pin stability / output cap per reference design",
    ("RST1", "HX 3X4X2-2P-1.6N TACTILE SWITCH"): "Tactile: strap / reset function per schematic",
    ("R12", "22R"): "USB D− series resistor between module and connector",
    ("C5", "2.2uF"): "ESP32-C3 module / 3V3 rail local bypass (per placement)",
    ("U2", "TLV75733PDBVR"): "3.3 V LDO: up to ~1 A out, low IQ; feeds digital + RF rail",
    ("R17", "4.7k"): "BNO086 strap / support network (per schematic)",
    ("R6", "220R"): "RGB LED blue cathode resistor (IO3); tuned with R7/R8 for TZ-P4-1615RGBTCA1 balance on 3V3",
    ("PWR1", "K3-1280S-K1"): "Slide switch: battery disconnect / power control",
    ("R8", "68R"): "RGB LED green cathode resistor (IO1)",
    ("C10", "0.1uf"): "3V3 / LDO output HF decoupling",
    ("C3", "100nf"): "USB_5V (post-fuse VBUS) HF bypass to GND",
    ("U3", "TP4054_C5381776"): "Linear Li-ion charger from USB_5V; CC/CV, ~500 mA with R4",
    ("BUZZER1", "MLT-5020"): "Passive magnetic buzzer: PWM via Q2 low-side switch",
    ("R10", "5.1K"): "USB-C CC2 pulldown (Rd) for default current advertisement",
    ("R1", "220k"): "Battery divider high side: VBAT_SW to ADC tap on IO4 (with R2)",
    ("GND_0", "DNP"): "Optional GND test pad — do not populate by default",
    ("R9", "5.1K"): "USB-C CC1 pulldown (Rd)",
    ("R20", "330R"): "Q2 gate series resistor: limits inrush / EMI from IO21",
    ("C12", "100nf"): "LDO / 3V3 bypass (per net)",
    ("F1", "0805L050WR"): "PTC resettable fuse ~500 mA hold on battery feed (matches charge budget)",
    ("C1", "10uf"): "VBAT bulk after switch: local energy for TX bursts",
    ("FUNC1", "HX 3X4X2-2P-1.6N TACTILE SWITCH"): "User button: FUNC to GND, IO7 with internal pull-up",
    ("LED1", "TZ-P4-1615RGBTCA1-0.55T"): "1615 RGB common-anode: cathodes via R6–R8; active-low drive on IO0/1/3",
    ("R16", "4.7k"): "BNO086 strap / support network (per schematic)",
    ("C9", "10uf"): "3V3 bulk decoupling",
    ("C8", "10uF"): "LDO input or output bulk (per schematic net)",
    ("Q2", "AO3400A"): "NMOS low-side: PWM buzzer − terminal; Id rating >> buzzer current",
    ("Q3", "AO3401A"): "PMOS reverse-polarity protection on battery input: source from JST+, drain to VBAT rail",
    ("R22", "100k"): "Q3 PMOS gate pull-down to GND: keeps reverse-protection FET enhanced with correct battery polarity",
    ("IC1", "BNO086"): "9-axis fusion IMU (accel/gyro/mag); SPI to ESP32; straps per schematic",
    ("R15", "10k"): "BNO086 CLKSEL0 pull-up to 3V3",
    ("R7", "100R"): "RGB LED red cathode resistor (IO0)",
    ("R19", "10k"): "Power control: TLV75733 EN / divider network (see schematic nets)",
    ("R13", "10k"): "BNO086 strap (NRST/BOOT network per schematic)",
}

UNIT_PRICE_BLOCK = """\
		(property "Unit Price (USD)" "{price}"
			(at 0 0 0)
			(effects
				(font
					(size 1.27 1.27)
				)
				(hide yes)
			)
		)
"""

def esc(s: str) -> str:
    return s.replace("\\", "\\\\").replace('"', '\\"')

def extract_ref_value(block: str) -> tuple[str | None, str]:
    rm = re.search(
        r'\(project "Azimuth"\s*\(path "[^"]*"\s*\(reference "([^"]+)"',
        block,
    )
    vm = re.search(r'\(property "Value" "([^"]*)"', block)
    ref = rm.group(1) if rm else None
    val = vm.group(1) if vm else ""
    return ref, val

def process_block(block: str) -> str:
    ref, val = extract_ref_value(block)
    if not ref or ref.startswith("#PWR"):
        return block

    key = (ref, val)
    if key in DESCS:
        nd = esc(DESCS[key])
        # Value is on the same line as (property "Description" "...") — no ")" on that line in KiCad 9
        block = re.sub(
            r'\(property "Description" "[^"]*"',
            f'(property "Description" "{nd}"',
            block,
            count=1,
        )

    lc = re.search(r'\(property "LCSC Part" "([^"]*)"', block)
    lcsc = lc.group(1) if lc else ""
    price = ""
    if lcsc and lcsc in UNIT_PRICE_USD:
        price = UNIT_PRICE_USD[lcsc]
    elif val in UNIT_PRICE_MPN:
        price = UNIT_PRICE_MPN[val]
    elif ref in ("BAT_0", "GND_0") and val == "DNP":
        price = "0"

    if price == "":
        return block

    # Update existing price field if present; otherwise insert before first pin.
    if '(property "Unit Price (USD)"' in block:
        return re.sub(
            r'(\(property "Unit Price \(USD\)" ")[^"]*(")',
            rf"\g<1>{price}\g<2>",
            block,
            count=1,
        )

    pin_m = re.search(r"\n\t\t\(pin ", block)
    if not pin_m:
        return block
    ins = UNIT_PRICE_BLOCK.format(price=price)
    return block[: pin_m.start()] + "\n" + ins + block[pin_m.start() :]

def main() -> None:
    lines = SCH.read_text().splitlines(keepends=True)
    out: list[str] = []
    i = 0
    while i < len(lines):
        if lines[i] == "\t(symbol\n" and i + 1 < len(lines) and "\t\t(lib_id" in lines[i + 1]:
            start = i
            depth = 0
            j = i
            while j < len(lines):
                depth += lines[j].count("(") - lines[j].count(")")
                j += 1
                if depth == 0:
                    break
            block = "".join(lines[start:j])
            out.append(process_block(block))
            i = j
        else:
            out.append(lines[i])
            i += 1
    SCH.write_text("".join(out))
    print(f"Updated {SCH.relative_to(REPO)}")

if __name__ == "__main__":
    main()

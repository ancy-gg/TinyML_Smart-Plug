import csv, random

def clip(x, lo, hi):
    return max(lo, min(hi, x))

def generate(path="pseudo_arc_dataset.csv", n=2000, seed=42):
    random.seed(seed)
    rows = []

    for _ in range(n):
        r = random.random()
        if r < 0.70:
            state = "normal"
        elif r < 0.85:
            state = "overload"
        elif r < 0.95:
            state = "heating"
        else:
            state = "arc"

        v_rms = random.gauss(220.0, 4.0)
        temp_c = random.gauss(35.0, 8.0)
        i_rms = abs(random.gauss(2.0, 1.5))

        if state == "overload":
            i_rms = clip(random.gauss(13.0, 2.5), 10.2, 22.0)
            temp_c = clip(random.gauss(55.0, 10.0), 30.0, 85.0)
            v_rms = clip(random.gauss(218.0, 6.0), 190.0, 240.0)

        if state == "heating":
            temp_c = clip(random.gauss(85.0, 10.0), 70.0, 120.0)
            i_rms = clip(abs(random.gauss(6.0, 3.0)), 0.2, 18.0)
            v_rms = clip(random.gauss(219.0, 5.5), 195.0, 240.0)

        if state == "arc":
            i_rms = clip(abs(random.gauss(7.0, 4.0)), 0.3, 18.0)
            v_rms = clip(random.gauss(210.0, 10.0), 160.0, 240.0)
            temp_c = clip(random.gauss(45.0, 12.0), 20.0, 95.0)

        if state == "normal":
            spectral_entropy = clip(random.gauss(0.35, 0.08), 0.05, 0.70)
            thd_pct = clip(abs(random.gauss(3.5, 1.5)), 0.0, 12.0)
            zcv = clip(abs(random.gauss(0.08, 0.04)), 0.0, 0.25)
        elif state == "overload":
            spectral_entropy = clip(random.gauss(0.45, 0.10), 0.10, 0.80)
            thd_pct = clip(abs(random.gauss(8.0, 4.0)), 1.0, 25.0)
            zcv = clip(abs(random.gauss(0.12, 0.06)), 0.02, 0.35)
        elif state == "heating":
            spectral_entropy = clip(random.gauss(0.40, 0.10), 0.10, 0.80)
            thd_pct = clip(abs(random.gauss(6.5, 3.0)), 1.0, 22.0)
            zcv = clip(abs(random.gauss(0.10, 0.05)), 0.01, 0.30)
        else:
            spectral_entropy = clip(random.gauss(0.82, 0.10), 0.45, 0.99)
            thd_pct = clip(abs(random.gauss(28.0, 12.0)), 10.0, 85.0)
            zcv = clip(abs(random.gauss(0.55, 0.18)), 0.20, 0.95)

        # Your stated arc logic:
        label_arc = 1 if (spectral_entropy > 0.75 or (thd_pct > 20.0 and zcv > 0.35)) else 0

        # small label noise
        if random.random() < 0.02:
            label_arc = 1 - label_arc

        rows.append([spectral_entropy, thd_pct, zcv, v_rms, i_rms, temp_c, label_arc, state])

    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["spectral_entropy","thd_pct","zcv","v_rms","i_rms","temp_c","label_arc","state"])
        w.writerows(rows)

    print(f"Wrote {n} rows to {path}")

if __name__ == "__main__":
    generate()

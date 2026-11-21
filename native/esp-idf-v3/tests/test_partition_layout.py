from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]

class PartitionLayoutTests(unittest.TestCase):
    def test_partition_ranges_do_not_overlap_and_fit_16mb(self):
        rows = []
        for raw in (ROOT / "partitions.csv").read_text().splitlines():
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            fields = [part.strip() for part in line.split(",")]
            name, kind, subtype, offset, size = fields[:5]
            start, length = int(offset, 0), int(size, 0)
            rows.append((name, start, start + length))
        rows.sort(key=lambda item: item[1])
        for previous, current in zip(rows, rows[1:]):
            self.assertLessEqual(previous[2], current[1], f"{previous[0]} overlaps {current[0]}")
        self.assertLessEqual(max(row[2] for row in rows), 16 * 1024 * 1024)

    def test_storage_partition_is_littlefs_slot(self):
        text = (ROOT / "partitions.csv").read_text()
        self.assertRegex(text, r"(?m)^storage\s*,\s*data\s*,\s*spiffs")

    def test_two_ota_slots_are_present(self):
        text = (ROOT / "partitions.csv").read_text()
        self.assertRegex(text, r"(?m)^ota_0\s*,\s*app\s*,\s*ota_0")
        self.assertRegex(text, r"(?m)^ota_1\s*,\s*app\s*,\s*ota_1")

if __name__ == "__main__":
    unittest.main()

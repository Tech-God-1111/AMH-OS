import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
FW = ROOT / "firmware"


class ProjectTests(unittest.TestCase):
    def test_required_files(self):
        for path in ("main.py", "boot.py", "amh/api.py", "amh/config.py",
                     "amh/wifi.py", "amh/webserver.py", "web/index.html",
                     "web/app.css", "web/app.js"):
            with self.subTest(path=path):
                self.assertTrue((FW / path).is_file())

    def test_legacy_build_files_absent(self):
        self.assertFalse((ROOT / "Makefile").exists())
        self.assertFalse((ROOT / "third_party" / "nuttx").exists())

    def test_gpio_disabled_by_default(self):
        source = (FW / "amh" / "hardware.py").read_text(encoding="utf-8")
        self.assertIn("ALLOWED_OUTPUT_PINS = ()", source)


if __name__ == "__main__":
    unittest.main()

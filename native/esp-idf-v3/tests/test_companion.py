import ast
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]

class CompanionStaticTests(unittest.TestCase):
    def test_companion_parses_and_has_no_shell_execution(self):
        source = (ROOT / "pc-companion" / "amh_companion.py").read_text()
        ast.parse(source)
        self.assertNotIn("os.system(", source)
        self.assertNotIn("shell=True", source)
        self.assertIn("127.0.0.1", source)
        self.assertIn("Authorization", source)

if __name__ == "__main__":
    unittest.main()

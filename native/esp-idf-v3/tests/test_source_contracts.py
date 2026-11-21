from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]

class SourceContractTests(unittest.TestCase):
    def test_nvs_recovery_does_not_erase_data_automatically(self):
        main = (ROOT / "main" / "main.c").read_text()
        self.assertIn("refusing automatic erase", main)
        self.assertNotIn("nvs_flash_erase(", main)

    def test_storage_is_not_autoformatted_on_mount(self):
        storage = (ROOT / "main" / "storage_service.c").read_text()
        self.assertIn(".format_if_mount_failed = false", storage)
        self.assertIn("FORMAT AMH-OS STORAGE", storage)
        self.assertIn("esp_vfs_fat_spiflash_format_rw_wl", storage)

    def test_file_api_rejects_traversal_and_backslashes(self):
        storage = (ROOT / "main" / "storage_service.c").read_text()
        self.assertGreaterEqual(storage.count('strstr(relative, "..")'), 1)
        self.assertIn("strchr(relative, '\\\\')", storage)
        self.assertIn('strstr(p, "..")', storage)

    def test_safe_mode_skips_network_and_ui_stages(self):
        main = (ROOT / "main" / "main.c").read_text()
        self.assertIn('{"wifi-ap", stage_wifi, true, true}', main)
        self.assertIn('{"desktop-http", stage_ui, true, true}', main)

if __name__ == "__main__":
    unittest.main()

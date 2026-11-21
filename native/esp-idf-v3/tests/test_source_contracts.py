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
        self.assertIn("esp_vfs_littlefs_register", storage)
        self.assertIn("esp_littlefs_format", storage)
        self.assertIn("esp_littlefs_info", storage)

    def test_file_api_rejects_traversal_and_backslashes(self):
        storage = (ROOT / "main" / "storage_service.c").read_text()
        self.assertGreaterEqual(storage.count('strstr(relative, "..")'), 1)
        self.assertIn("strchr(relative, '\\\\')", storage)
        self.assertIn('strstr(p, "..")', storage)

    def test_desktop_exposes_device_file_manager_and_safe_terminal(self):
        html = (ROOT / "main" / "web" / "index.html").read_text()
        for command in ("help", "version", "status", "uptime", "apps", "clear", "ls", "cat", "mkdir", "rm", "write"):
            self.assertIn("cmd==='" + command + "'", html)
        self.assertIn("Device File Explorer", html)
        self.assertIn("setupDeviceExplorer(w)", html)
        self.assertNotIn("eval(", html)
        self.assertNotIn("new Function(", html)

    def test_network_services_are_disabled_by_default(self):
        kconfig = (ROOT / "main" / "Kconfig.projbuild").read_text()
        sdkconfig = (ROOT / "sdkconfig.defaults").read_text()
        self.assertIn("config AMH_ENABLE_WEB_DESKTOP", kconfig)
        self.assertIn("default n", kconfig)
        self.assertNotIn("CONFIG_AMH_ENABLE_WEB_DESKTOP=y", sdkconfig)
        self.assertIn("CONFIG_ESP_CONSOLE_UART_BAUDRATE=115200", sdkconfig)

    def test_safe_mode_skips_network_and_ui_stages(self):
        main = (ROOT / "main" / "main.c").read_text()
        self.assertIn('{"wifi-ap", stage_wifi, true, true}', main)
        self.assertIn('{"desktop-http", stage_ui, true, true}', main)

if __name__ == "__main__":
    unittest.main()

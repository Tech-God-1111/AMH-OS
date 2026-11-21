from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]


def read_source(path: Path) -> str:
    """Read repository text files consistently, independent of Windows locale."""
    return path.read_text(encoding="utf-8")


class SourceContractTests(unittest.TestCase):
    def test_nvs_recovery_does_not_erase_data_automatically(self):
        main = read_source(ROOT / "main" / "main.c")
        self.assertIn("refusing automatic erase", main)
        self.assertNotIn("nvs_flash_erase(", main)

    def test_storage_is_not_autoformatted_on_mount(self):
        storage = read_source(ROOT / "main" / "storage_service.c")
        self.assertIn(".format_if_mount_failed = false", storage)
        self.assertIn("FORMAT AMH-OS STORAGE", storage)
        self.assertIn("esp_vfs_littlefs_register", storage)
        self.assertIn("esp_littlefs_format", storage)
        self.assertIn("esp_littlefs_info", storage)

    def test_file_api_rejects_traversal_and_backslashes(self):
        storage = read_source(ROOT / "main" / "storage_service.c")
        self.assertGreaterEqual(storage.count('strstr(relative, "..")'), 1)
        self.assertIn("strchr(relative, '\\\\')", storage)
        self.assertIn('strstr(p, "..")', storage)

    def test_desktop_exposes_device_file_manager_and_safe_terminal(self):
        html = read_source(ROOT / "main" / "web" / "index.html")
        for command in ("help", "version", "status", "uptime", "apps", "clear", "ls", "cat", "mkdir", "rm", "write"):
            self.assertIn("cmd==='" + command + "'", html)
        self.assertIn("Device File Explorer", html)
        self.assertIn("setupDeviceExplorer(w)", html)
        self.assertNotIn("eval(", html)
        self.assertNotIn("new Function(", html)

    def test_network_services_are_disabled_by_default(self):
        kconfig = read_source(ROOT / "main" / "Kconfig.projbuild")
        sdkconfig = read_source(ROOT / "sdkconfig.defaults")
        self.assertIn("config AMH_ENABLE_WEB_DESKTOP", kconfig)
        self.assertIn("default n", kconfig)
        self.assertNotIn("CONFIG_AMH_ENABLE_WEB_DESKTOP=y", sdkconfig)
        self.assertIn("CONFIG_ESP_CONSOLE_UART_BAUDRATE=115200", sdkconfig)

    def test_safe_mode_skips_network_and_ui_stages(self):
        main = read_source(ROOT / "main" / "main.c")
        self.assertIn('{"wifi-ap", stage_wifi, true, true}', main)
        self.assertIn('{"desktop-http", stage_ui, true, true}', main)


if __name__ == "__main__":
    unittest.main()

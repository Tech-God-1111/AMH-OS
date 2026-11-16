#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include <sys/wait.h>
#include <unistd.h>
#include <malloc.h>

#define AMH_VERSION "0.5.0-alpha"

static void show_help(void)
{
  printf("\nAMH-OS %s - ESP32-S3 shell\n", AMH_VERSION);
  printf("Usage: amh <command> [args]\n");
  printf("  help          Show commands\n");
  printf("  sys           Uptime, chip target and heap statistics\n");
  printf("  wifi          Wi-Fi setup/status guidance\n");
  printf("  fs [path]     List a directory and filesystem capacity\n");
  printf("  apps          List supported native utilities\n");
  printf("  run <app>     Launch an allow-listed utility\n\n");
}

static void show_system(void)
{
  struct timespec ts;
  unsigned long long uptime = 0;
  printf("\nAMH-OS system monitor\n---------------------\n");
  printf("Release          : %s\n", AMH_VERSION);
  printf("Target           : ESP32-S3 / Xtensa LX7\n");
  printf("CPU cores        : 2 (hardware capability)\n");
  printf("Nominal clock    : 240 MHz target; runtime clock API not wired\n");
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
    uptime = (unsigned long long)ts.tv_sec;
  printf("Uptime           : %llu seconds\n", uptime);
#ifdef CONFIG_MM_MALLINFO
  {
    struct mallinfo mi = mallinfo();
    printf("Heap allocated   : %d bytes\n", mi.uordblks);
    printf("Heap free        : %d bytes\n", mi.fordblks);
    printf("Heap arena       : %d bytes\n", mi.arena);
  }
#else
  printf("Heap statistics  : enable CONFIG_MM_MALLINFO\n");
#endif
  printf("External PSRAM   : not exposed by generic board profile\n");
  printf("CPU utilization : requires board-specific scheduler telemetry\n\n");
}

static void show_wifi(void)
{
  printf("\nAMH-OS Wi-Fi management\n");
  printf("Enable the ESP32-S3 Wi-Fi driver, network initialization and WAPI\n");
  printf("utilities in NuttX menuconfig. Then use native NSH commands:\n");
  printf("  ifconfig\n");
  printf("  wapi scan wlan0              (if scan is supported)\n");
  printf("  wapi psk wlan0 <key> 3       (session configuration)\n");
  printf("  wapi essid wlan0 <ssid> 1\n");
  printf("  renew wlan0\n");
  printf("  ifconfig wlan0\n");
  printf("Never commit real Wi-Fi credentials to source control.\n\n");
}

static int list_fs(const char *path)
{
  DIR *dir = opendir(path);
  struct dirent *entry;
  struct statvfs st;
  unsigned int count = 0;
  if (!dir)
    {
      printf("Cannot open directory: %s\n", path);
      return 1;
    }
  printf("\nDirectory: %s\n", path);
  while ((entry = readdir(dir)) != NULL)
    {
      if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
          printf("  %s\n", entry->d_name);
          count++;
        }
    }
  closedir(dir);
  printf("%u entries\n", count);
  if (statvfs(path, &st) == 0)
    {
      printf("Total bytes : %llu\n",
        (unsigned long long)st.f_blocks * st.f_frsize);
      printf("Free bytes  : %llu\n",
        (unsigned long long)st.f_bfree * st.f_frsize);
    }
  else
    printf("Filesystem capacity not available for this path.\n");
  return 0;
}

static int allowed_app(const char *name)
{
  static const char *const apps[] = {
    "uname", "ps", "free", "ls", "df", "ifconfig", "wapi",
    "ping", "mount", "umount", "date", "uptime", NULL
  };
  int i;
  for (i = 0; apps[i]; i++)
    if (!strcmp(name, apps[i])) return 1;
  return 0;
}

static void show_apps(void)
{
  printf("\nAMH-OS native utility allow-list\n");
  printf("uname ps free ls df ifconfig wapi ping mount umount date uptime\n");
  printf("Launch: amh run <utility> [arguments]\n");
  printf("Availability depends on the NuttX build configuration.\n\n");
}

static int run_app(int argc, char **argv)
{
  pid_t pid;
  int status;
  if (argc < 3)
    {
      printf("Usage: amh run <allow-listed-utility> [args]\n");
      return 1;
    }
  if (!allowed_app(argv[2]))
    {
      printf("Blocked: '%s' is not in the AMH utility allow-list.\n", argv[2]);
      return 1;
    }
  pid = fork();
  if (pid < 0) { perror("fork"); return 1; }
  if (pid == 0)
    {
      execvp(argv[2], &argv[2]);
      perror("execvp");
      _exit(127);
    }
  if (waitpid(pid, &status, 0) < 0) { perror("waitpid"); return 1; }
  return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

int main(int argc, FAR char *argv[])
{
  if (argc < 2 || !strcmp(argv[1], "help"))
    { show_help(); return 0; }
  if (!strcmp(argv[1], "sys")) { show_system(); return 0; }
  if (!strcmp(argv[1], "wifi")) { show_wifi(); return 0; }
  if (!strcmp(argv[1], "fs")) return list_fs(argc > 2 ? argv[2] : "/");
  if (!strcmp(argv[1], "apps")) { show_apps(); return 0; }
  if (!strcmp(argv[1], "run")) return run_app(argc, argv);
  printf("Unknown AMH-OS command: %s (try 'amh help')\n", argv[1]);
  return 1;
}

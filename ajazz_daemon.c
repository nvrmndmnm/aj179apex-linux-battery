#include <libusb-1.0/libusb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define INTERFACE_NUM 2
#define OUTPUT_FILE "/tmp/ajazz_battery"

libusb_context *ctx = NULL;
int running = 1;

int get_battery_from_device(libusb_device_handle *dev_handle);
void write_status(int bat_level, const char *class_name);

void handle_sig(int sig) {
  (void)sig;
  running = 0;
}

int main(int argc, char *argv[]) {
  signal(SIGINT, handle_sig);
  signal(SIGTERM, handle_sig);

  uint16_t vendor_id = 0x3151;
  uint16_t product_id = 0x5007;

  if (argc >= 3) {
    vendor_id = (uint16_t)strtol(argv[1], NULL, 16);
    product_id = (uint16_t)strtol(argv[2], NULL, 16);
  } else {
    printf("No IDs provided, defaulting to 8K Dongle: %04x:%04x\n", vendor_id, product_id);
  }

  printf("Ajazz 8K Daemon started. Looking for %04x:%04x...\n", vendor_id, product_id);
  
  if (libusb_init(&ctx) < 0)
    return 1;

  write_status(-1, "disconnected");

  while (running) {
    libusb_device_handle *handle =
        libusb_open_device_with_vid_pid(ctx, vendor_id, product_id);

    if (handle) {
      int drv_active = libusb_kernel_driver_active(handle, INTERFACE_NUM);
      if (drv_active == 1) {
        libusb_detach_kernel_driver(handle, INTERFACE_NUM);
      }
      int claim_res = libusb_claim_interface(handle, INTERFACE_NUM);
      if (claim_res >= 0) {
        int bat = get_battery_from_device(handle);

        if (bat >= 0) {
          write_status(bat, "mouse");
        }
        libusb_release_interface(handle, INTERFACE_NUM);
      }

      if (drv_active == 1) {
        libusb_attach_kernel_driver(handle, INTERFACE_NUM);
      }
      libusb_close(handle);
    } else {
      // Mouse is sleeping/unplugged.
      // We do NOT write "..." here. We keep the last known battery %
    }

    usleep(1500000);
  }

  libusb_exit(ctx);
  return 0;
}

int get_battery_from_device(libusb_device_handle *handle) {
  unsigned char f7_payload[64] = {0};
  f7_payload[0] = 0xf7;
  int r1 = libusb_control_transfer(handle, 0x21, 0x09, 0x0300, INTERFACE_NUM,
                                   f7_payload, 64, 1000);
  if (r1 < 0) {
    return -1;
  }
  usleep(15000);

  unsigned char out_buf[64] = {0};
  int r2 = libusb_control_transfer(handle, 0xA1, 0x01, 0x0300, INTERFACE_NUM,
                                   out_buf, 64, 1000);
  if (r2 < 0) {
    return -1;
  }
  
  return out_buf[2];
}

void write_status(int bat_level, const char *class_name) {
  (void)class_name;

  FILE *f = fopen(OUTPUT_FILE, "w");
  if (f) {
    if (bat_level >= 0)
      fprintf(f, "%d\n", bat_level);
    else
      fprintf(f, "Disconnected\n");
    fclose(f);
  }
}

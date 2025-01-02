#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define IFNAMSIZ 16 // Define fallback for non-Linux systems
#else
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/if.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cerrno>
#include <cstdint>
#endif

using namespace std;

// ARP header
struct arp_hdr {
    uint16_t hwtype;
    uint16_t protype;
    unsigned char hwsize;
    unsigned char prosize;
    uint16_t opcode;
    unsigned char data[];
} __attribute__((packed));

// ARP IPv4 payload
struct arp_ipv4 {
    unsigned char smac[6];
    uint32_t sip;
    unsigned char dmac[6];
    uint32_t dip;
} __attribute__((packed));

// Ethernet header
struct eth_hdr {
    unsigned char dmac[6];
    unsigned char smac[6];
    uint16_t ethertype;
    unsigned char payload[];
} __attribute__((packed));

// Function prototypes
int tun_alloc(char* dev);
void print_error(const char* msg);
int tun_read(unsigned char* buf, size_t buflen);
struct eth_hdr* init_eth_hdr(unsigned char* buf);
void handle_frame(void* netdev, struct eth_hdr* hdr);

// Buffer size for TUN device
#define BUFLEN 1500

int tun_alloc(char* dev) {
#ifdef _WIN32
    cerr << "TUN/TAP allocation is not supported on Windows." << endl;
    return -1; // Return error on unsupported platforms
#else
    struct ifreq ifr{};
    int fd, err;

    // Open TUN/TAP device
    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        print_error("Cannot open TUN/TAP dev");
        exit(1);
    }

    // Initialize ifreq structure
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (*dev) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ - 1);
    }

    // Set the TUN device
    if ((err = ioctl(fd, TUNSETIFF, (void*)&ifr)) < 0) {
        print_error("ERR: Could not ioctl tun");
        close(fd);
        return err;
    }

    unsigned char buf[BUFLEN];

    // Read from TUN device
    if (tun_read(buf, BUFLEN) < 0) {
        print_error("ERR: Read from tun_fd");
    }

    // Initialize Ethernet header
    struct eth_hdr* hdr = init_eth_hdr(buf);

    // Process the frame (mock example)
    handle_frame(nullptr, hdr);

    // Store the interface name
    strncpy(dev, ifr.ifr_name, IFNAMSIZ - 1);
    return fd;
#endif
}

void print_error(const char* msg) {
#ifdef _WIN32
    cerr << msg << endl; // Simple error message for Windows
#else
    cerr << msg << ": " << strerror(errno) << endl; // Linux-specific error with errno
#endif
}

int tun_read(unsigned char* buf, size_t buflen) {
    // Mock implementation for tun_read
    memset(buf, 0, buflen); // Fill buffer with zeros (dummy data)
    return 0; // Success
}

struct eth_hdr* init_eth_hdr(unsigned char* buf) {
    return (struct eth_hdr*)buf; // Simple cast to interpret as Ethernet header
}

void handle_frame(void* netdev, struct eth_hdr* hdr) {
    // Mock frame handling (just print ethertype)
    cout << "Processing Ethernet frame with Ethertype: " << hdr->ethertype << endl;
}

int main() {
    char dev[IFNAMSIZ] = {0}; // Declare dev properly
    int fd = tun_alloc(dev);
    if (fd > 0) {
        cout << "TUN/TAP device allocated: " << dev << endl;
    }
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#if defined(__FreeBSD_kernel__) || defined(__APPLE__)
        #include <net/if.h>
#endif

#if defined(__APPLE__)
#include <netdb.h>
#include <net/if_dl.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <net/if_types.h>  // Include this for IFT_ETHER

struct if_stat *get_stats(const char *iface);
#endif

#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

struct if_stat {
    char iface_name[IFNAMSIZ];
    unsigned long long packets_received;
    unsigned long long packets_sent;
    unsigned long long bytes_received;
    unsigned long long bytes_sent;
    unsigned long long input_errors;  // Add this field
    unsigned long long output_errors; // Ensure this field exists as well

	unsigned long long in_packets, in_bytes, in_errors, in_drops;
	unsigned long long in_fifo, in_frame, in_compress, in_multicast;
	unsigned long long out_bytes, out_packets, out_errors, out_drops;
	unsigned long long out_fifo, out_colls, out_carrier, out_multicast;
};


// Function to gather network statistics
struct if_stat *get_stats(const char *iface) {
    struct ifaddrs *ifap, *ifa;
    struct if_data *if_data;
    struct if_stat *ifstat;

    ifstat = malloc(sizeof(struct if_stat));
    if (ifstat == NULL) {
        perror("malloc");
        return NULL;
    }

    if (getifaddrs(&ifap) != 0) {
        perror("getifaddrs");
        free(ifstat);
        return NULL;
    }

    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_LINK) {
            if (strcmp(ifa->ifa_name, iface) == 0) {
                if_data = (struct if_data *)ifa->ifa_data;
                if (if_data != NULL) {
                    strncpy(ifstat->iface_name, iface, IFNAMSIZ - 1);
                    ifstat->iface_name[IFNAMSIZ - 1] = '\0';
                    ifstat->packets_received = if_data->ifi_ipackets;
                    ifstat->packets_sent = if_data->ifi_opackets;
                    ifstat->bytes_received = if_data->ifi_ibytes;
                    ifstat->bytes_sent = if_data->ifi_obytes;
                    ifstat->in_errors = if_data->ifi_ierrors;
                    ifstat->output_errors = if_data->ifi_oerrors;
                    break;
                }
            }
        }
    }

    freeifaddrs(ifap);
    return ifstat;
}


int main() {
    struct if_stat *stat = get_stats("en0");  // Replace "en0" with your network interface

    if (stat != NULL) {
        printf("Interface: %s\n", stat->iface_name);
        printf("Packets received: %llu\n", stat->packets_received);
        printf("Packets sent: %llu\n", stat->packets_sent);
        printf("Bytes received: %llu\n", stat->bytes_received);
        printf("Bytes sent: %llu\n", stat->bytes_sent);
        printf("Input errors: %llu\n", stat->input_errors);
        printf("Output errors: %llu\n", stat->output_errors);
        free(stat);  // Don't forget to free the allocated memory
    } else {
        printf("Failed to get network statistics.\n");
    }

    return 0;
}



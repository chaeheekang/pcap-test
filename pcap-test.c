#include <pcap.h>
#include <stdbool.h>
#include <stdio.h>
#include <libnet.h>

void usage() {
	printf("syntax: pcap-test <interface>\n");
	printf("sample: pcap-test wlan0\n");
}

typedef struct {
	char* dev_;
} Param;

Param param  = {
	.dev_ = NULL
};

bool parse(Param* param, int argc, char* argv[]) {
	if (argc != 2) {
		usage();
		return false;
	}
	param->dev_ = argv[1];
	return true;
}

int main(int argc, char* argv[]) {
	if (!parse(&param, argc, argv))
		return -1;

	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t* pcap = pcap_open_live(param.dev_, BUFSIZ, 1, 1000, errbuf);
	if (pcap == NULL) {
		fprintf(stderr, "pcap_open_live(%s) return null - %s\n", param.dev_, errbuf);
		return -1;
	}

	while (true) {
		struct pcap_pkthdr* header;
		const u_char* packet;
		int res = pcap_next_ex(pcap, &header, &packet);
		if (res == 0) continue;
		if (res == PCAP_ERROR || res == PCAP_ERROR_BREAK) {
			printf("pcap_next_ex return %d(%s)\n", res, pcap_geterr(pcap));
			break;
		}
        //struct
        struct libnet_ethernet_hdr *eth;
        struct libnet_ipv4_hdr *ipv4;
        struct libnet_tcp_hdr *tcp;

        //1.Ethernet Header
        eth = (struct libnet_ethernet_hdr *)packet;
        //check ipv4
        if(ntohs(eth->ether_type)!=0x0800){
            //printf("This is not ipv4");
            continue;
        }
        //2.IP Header
        ipv4 = (struct libnet_ipv4_hdr*)(packet+sizeof (*eth));
        //3.TCP Header
        tcp= (struct libnet_tcp_hdr *)(packet+sizeof (*eth)+sizeof(*ipv4));
        //check tcp
        if(ipv4->ip_p != 6){
            //printf("This is not tcp!");
            continue;
        }
        //print 1
        printf("src mac: %02x %02x %02x %02x %02x %02x\n",eth->ether_shost[0],eth->ether_shost[1],eth->ether_shost[2],eth->ether_shost[3],eth->ether_shost[4],eth->ether_shost[5]);
        printf("dst mac: %02x %02x %02x %02x %02x %02x\n",eth->ether_dhost[0],eth->ether_dhost[1],eth->ether_dhost[2],eth->ether_dhost[3],eth->ether_dhost[4],eth->ether_dhost[5]);

        //print 2
        printf("src ip: %s\n",inet_ntoa(ipv4->ip_src));
        printf("dst ip: %s\n",inet_ntoa(ipv4->ip_dst));

        //print 3
        printf("src port: %d\n",ntohs(tcp->th_sport));
        printf("dst port: %d\n", ntohs(tcp->th_dport));

        //4.Payload Data
        int size=header->caplen-54;
        if (size==0){
            printf("NO DATA\n");
        }
        else if(size<10){
            printf("Payload: ");
            for(int i=0;i<size;i++){
                printf("%02x ",packet[53+i]);
            }
            printf("\n");
        }
        else{
            printf("Payload: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",packet[53],packet[54],packet[55],packet[56],packet[57],
                   packet[58],packet[59],packet[60],packet[61],packet[62] );
        }
        printf("============================================\n");
    }

	pcap_close(pcap);
}

#include <stdio.h>
#include <inttypes.h>
#include "net/gnrc.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/udp.h"
#include "net/gnrc/pktdump.h"
#include "timex.h"
#include "xtimer.h"
#include "config.h"

void udp_envia(ipv6_addr_t addr, uint8_t *porta, char *data)
{
    gnrc_pktsnip_t *payload, *udp, *ip;
  
    /* allocate payload */
    if( (payload = gnrc_pktbuf_add(NULL, data, strlen(data), GNRC_NETTYPE_UNDEF)) == NULL){
            puts("Erro ao copiar mensagem para buffer do pacote");
            return;
    }

    /* allocate UDP header, set source port := destination port */
    if( (udp = gnrc_udp_hdr_build(payload, porta, 2, porta, 2)) == NULL){
            puts("Erro ao alocar header UDP");
            gnrc_pktbuf_release(payload);
            return;
    }

    /* allocate IPv6 header */
    if( (ip = gnrc_ipv6_hdr_build(udp, NULL, 0, (uint8_t *)&addr, sizeof(addr))) == NULL){
            puts("Erro ao alocar header IPv6");
            gnrc_pktbuf_release(udp);
            return;
    }

    /* send packet */
    if (!gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL, ip)) {
            puts("Error: unable to locate UDP thread");
            gnrc_pktbuf_release(ip);
            return;
    }

    printf("Success: send %u byte to [%s]:%u\n", (unsigned)payload->size, UDP_TARGET, UDP_TARGET_PORT);
}

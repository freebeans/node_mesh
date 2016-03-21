#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "msg.h"
#include "net/gnrc/netif.h"
#include "net/ipv6/addr.h"
#include "net/gnrc/ipv6/netif.h"
#include "net/gnrc/ipv6/nc.h"
#include "net/gnrc/rpl.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "config.h"


kernel_pid_t interface_detecta(void);
void interface_configura(kernel_pid_t);
void strcat_ip_global(char *, gnrc_ipv6_netif_t *);
void udp_envia(ipv6_addr_t, uint8_t *, char *);


#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];


int main(void)
{
	/* we need a message queue for the thread running the shell in order to
	 * receive potentially fast incoming networking packets */
	msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
	puts("Mesh Node AWGES");
 
	kernel_pid_t interface = interface_detecta();
	printf("Interface Wifi: %d\n\n", interface);
	interface_configura(interface);
    
    
    

    /* Usuário escolhe Shell/UDP */ {	
		printf("Aguardando usuário... ");
		/* inicializa GPIO do botão como entrada com PULL-UP */
		gpio_init(BUTTON_GPIO, GPIO_IN_PU);
		

		/* acende o LED e espera por 3s */
		LED0_ON;
		xtimer_usleep(1000000);
		LED0_OFF;
		
		/* se o usuario pressionar o botão, o programa cai no shell */
		if(!gpio_read(BUTTON_GPIO)){
			puts(" abrindo o shell.");
			char line_buf[SHELL_DEFAULT_BUFSIZE];
			shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
		}
		
		/* se o usuario não apertar o botão, começa a cuspir pacotes */
		xtimer_usleep(1000000);
		LED0_ON;
		xtimer_usleep(500000);
		LED0_OFF;
		
		
		puts(" enviando pacotes.");
    }
    
    /* obtenção do endereço de hardware (curto) - parte 1*/
    uint8_t hwaddr[8] = { 0 };
    int res = gnrc_netapi_get(interface, NETOPT_ADDRESS, 0, hwaddr, sizeof(hwaddr));
    char hwaddr_str[res * 3];
    
    /* string a ser enviada */
    char msg[100];

    /* handler da interface */
    gnrc_ipv6_netif_t *interface_handler;


    /* parse destination address */
    ipv6_addr_t destino_endereco;
    ipv6_addr_from_str(&destino_endereco, UDP_TARGET);

    uint8_t porta[2] = { (uint8_t) UDP_TARGET_PORT, (((uint16_t) UDP_TARGET_PORT) >> 8) };

    while(1){
		LED0_TOGGLE;
		msg[0] = '\0';
		
		/* obtem handler da interface */
		interface_handler = gnrc_ipv6_netif_get(interface);
		
		strcat(msg, "Hello, I'm ");
		
		/* obtenção do endereço de hardware (curto) - parte 2*/
		/* adiciona endereço (curto) à mensagem */
		strcat(msg, gnrc_netif_addr_to_str(hwaddr_str, sizeof(hwaddr_str), hwaddr, res));
		
		/* adiciona IPv6 global à mensagem e finaliza com nova linha */
		strcat_ip_global(msg, interface_handler);
		strcat(msg, "\n\n");
    
		/* envia 5 mensagens UDP para UDP_TARGET na porta UDP_TARGET_PORT */
		for(int i=0; i<5; i++){
			udp_envia(destino_endereco, porta, msg);
			xtimer_usleep(1000000);
		}
    }
    
    return 0;
}

kernel_pid_t interface_detecta(void){

    kernel_pid_t get_iface[GNRC_NETIF_NUMOF];
    size_t numof = gnrc_netif_get(get_iface);

    // itera sobre as interfaces retornadas
    for(size_t i=0; i<numof; i++){

	// recebe estrutura relativa à interface
	gnrc_ipv6_netif_t *entry = gnrc_ipv6_netif_get(get_iface[i]);

	// testa se a interface é cabeada
	if( !(entry->flags & GNRC_IPV6_NETIF_FLAGS_IS_WIRED) )
	    return get_iface[i];
    }
    return 0;
}

void interface_configura(kernel_pid_t interface){

	{	/* ifconfig [WIRELESS] set channel WIRELESS_CHANNEL */
		unsigned int res = WIRELESS_CHANNEL;
		gnrc_netapi_set(interface, NETOPT_CHANNEL, 0, (uint16_t *)&res, sizeof(uint16_t));
	}
	
	{	/* ifconfig [WIRELESS] set pan_id PANID */
		unsigned int res = PANID;
		gnrc_netapi_set(interface, NETOPT_NID, 0, (uint16_t *)&res, sizeof(uint16_t));
	}

	{	/* rpl init [WIRELESS] */
		puts("Iniciando RPL na interface sem fio");
		gnrc_rpl_init(interface);
	}
}

void strcat_ip_global(char * msg, gnrc_ipv6_netif_t *interface_handler){
	
	/* string para armazenar IPv6 */
	char ipv6_addr[IPV6_ADDR_MAX_STR_LEN];
	
	/* itera por todos os endereços IPv6 da interface representada pelo handler "interface_handler" */
	for (int i = 0; i < GNRC_IPV6_NETIF_ADDR_NUMOF; i++) {
        if (!ipv6_addr_is_unspecified(&interface_handler->addrs[i].addr)) {

			/* se conseguir converter endereço em string */
            if (ipv6_addr_to_str(ipv6_addr, &interface_handler->addrs[i].addr, IPV6_ADDR_MAX_STR_LEN)) {
				
				/* só queremos endereços que não sejam locais */
                if (!(ipv6_addr_is_link_local(&interface_handler->addrs[i].addr))) {
                    strcat(msg, "\n");
                    strcat(msg, ipv6_addr);
                }
            }
        }
    }
}

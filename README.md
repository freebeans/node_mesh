# gnrc_networking example

This example shows you how to try out the code in two different ways: Either by communicating
between the RIOT machine and its Linux host, or by communicating between two RIOT instances.
Note that the former only works with native, i.e. if you run RIOT on your Linux machine.

## Mudanças

Detecção e inicialização automática do RPL na interface sem fio. Logo após reset, se o botão
for pressionado, o programa cai no shell. Caso contrário, começa a enviar pacotes UDP para
um endereço definido no início do arquivo. São enviados 5 pacotes por segundo, e a cada
segundo a interface é varrida para buscar o endereço IPv4 global. A cada varredura, o LED
da placa muda de estado. Logo, LED piscando à uma taxa de 1Hz é uma indicação de que a placa
está enviando pacotes (ou ao menos tentando).

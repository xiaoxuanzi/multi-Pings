obj=pings_rbtree.o pings_checksum.o pings_memory.o pings_send_packets.o  pings_node_tree.o pings_print.o pings.o pings_recv_packets.o pings_init.o pings_event.o pings_packet.o
CC=gcc -Wall #-pedantic
pings:$(obj)
	$(CC) $(obj) -o pings
rbtree.o : pings_rbtree.h pings_rbtree.c
ping.o : pings.c pings.h
icmptree.o:pings_node_tree.c pings.h
checksum.o:pings_checksum.c pings.h
memory.o:pings_memory.c pings.h
send_packet.o:pings_send_packets.c pings.h
recv_packet.o:pings_recv_packets.c pings.h
print.o:pings_print.c pings.h
init.o:pings_init.c pings.h
ping_epoll.o:pings_event.c pings.h
packet.o:pings_packet.c pings.h

.INTERMEDIATE:$(obj)
#.PHONY:clean
#
#clean:
#	rm *.o

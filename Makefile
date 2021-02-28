all: isp producer consumer

producer: producer.c
	gcc -Wall -g -o producer producer.c

consumer: consumer.c
	gcc -Wall -g -o consumer consumer.c

isp: isp.c
	gcc -g -o isp isp.c

clean:
	rm -fr isp producer consumer *dSYM
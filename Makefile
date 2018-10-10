TLS12_CLIENT = tls12_rm_cert1_client
TLS12_SERVER = tls12_rm_cert1_server
TLS12_CLIENT2 = tls12_rm_cert2_client
TLS12_SERVER2 = tls12_rm_cert2_server
TARGET=$(TLS12_CLIENT) $(TLS12_SERVER) $(TLS12_CLIENT2) $(TLS12_SERVER2)

ifeq ($(OSSL_PATH),)
OPENSSL_PATH=../openssl
else
OPENSSL_PATH=$(OSSL_PATH)
endif

CFLAGS = -g -ggdb -Wall -Werror -I $(OPENSSL_PATH)/include
LDFLAGS = -L ./ -lssl -lcrypto -lpthread -ldl

CC = gcc
CP = cp
RM = rm

#.PHONY all init_task clean

all : init_task $(TARGET)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

init_task:
	@$(CP) $(OPENSSL_PATH)/libcrypto.a .
	@$(CP) $(OPENSSL_PATH)/libssl.a .

$(TLS12_CLIENT):$(TLS12_CLIENT).o
	$(CC) $^ $(LDFLAGS) -o $@

$(TLS12_SERVER):$(TLS12_SERVER).o
	$(CC) $^ $(LDFLAGS) -o $@

$(TLS12_CLIENT2):$(TLS12_CLIENT2).o
	$(CC) $^ $(LDFLAGS) -o $@

$(TLS12_SERVER2):$(TLS12_SERVER2).o
	$(CC) $^ $(LDFLAGS) -o $@

clean:
	@$(RM) -rf *.o *.a
	@$(RM) -rf $(TARGET)

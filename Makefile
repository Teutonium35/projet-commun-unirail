all: evc rbc

CLIENT_SSH_IP1 = 192.168.1.173
CLIENT_SSH_IP2 = 192.168.1.172

CLIENT_SSH_USER = pi
CLIENT_SSH_PASS = raspberry
CLIENT_SSH_PATH = /home/pi/Unirail-25

evc:
	$(MAKE) -C app/EVC

rbc:
	$(MAKE) -C app/RBC

clean:
	$(MAKE) -C app/EVC clean
	$(MAKE) -C app/RBC clean

install:
	sshpass -p $(CLIENT_SSH_PASS) scp -r ./EVC/* $(CLIENT_SSH_USER)@$(CLIENT_SSH_IP1):$(CLIENT_SSH_PATH)
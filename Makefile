all: evc rbc

EVC_SSH_IPS = 192.168.1.174 192.168.1.173 192.168.1.172

RBC_SSH_IP = 192.168.1.148

SSH_USER = pi
SSH_PASS = raspberry
SSH_PATH = /home/pi/Unirail-25

RBC_RUN_PORT = 3000

evc:
	$(MAKE) -C app/EVC

rbc:
	$(MAKE) -C app/RBC

clean:
	$(MAKE) -C app/EVC clean
	$(MAKE) -C app/RBC clean

install: install-evc-1 install-evc-2 install-evc-3 install-rbc

install-rbc:
	sshpass -p $(SSH_PASS) scp -r ./app $(SSH_USER)@$(RBC_SSH_IP):$(SSH_PATH)
	sshpass -p $(SSH_PASS) scp -r ./Makefile $(SSH_USER)@$(RBC_SSH_IP):$(SSH_PATH)
	sshpass -p $(SSH_PASS) ssh $(SSH_USER)@$(RBC_SSH_IP) "export TERM=xterm; cd $(SSH_PATH) && make clean && make rbc && clear"

install-evc-%:
	sshpass -p $(SSH_PASS) scp -r ./app $(SSH_USER)@$(word $*, $(EVC_SSH_IPS)):$(SSH_PATH)
	sshpass -p $(SSH_PASS) scp -r ./Makefile $(SSH_USER)@$(word $*, $(EVC_SSH_IPS)):$(SSH_PATH)
	sshpass -p $(SSH_PASS) ssh $(SSH_USER)@$(word $*, $(EVC_SSH_IPS)) "export TERM=xterm; cd $(SSH_PATH) && make clean && make evc && clear"

run-rbc: install-rbc
	sshpass -p $(SSH_PASS) ssh -t $(SSH_USER)@$(RBC_SSH_IP) "cd $(SSH_PATH) && ./app/RBC/bin/rbc $(RBC_RUN_PORT)"

run-evc-%: install-evc-%
	sshpass -p $(SSH_PASS) ssh -t $(SSH_USER)@$(word $*, $(EVC_SSH_IPS)) "cd $(SSH_PATH) && ./app/EVC/bin/evc $* $(RBC_SSH_IP) $(RBC_RUN_PORT)"

test-rbc: install-rbc
	sshpass -p $(SSH_PASS) ssh -t $(SSH_USER)@$(RBC_SSH_IP) "cd $(SSH_PATH) && make test"

test-evc-%: install-evc-%
	sshpass -p $(SSH_PASS) ssh -t $(SSH_USER)@$(word $*, $(EVC_SSH_IPS)) "cd $(SSH_PATH) && make test"
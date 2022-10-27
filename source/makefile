all:
	$(MAKE) -C client-adv;

.PHONY:clean
clean:
	rm -f `ls -F -R | grep '*' | sed 's/[/*]//g'`
	$(MAKE) -C client-adv clean;
http:http.c
	gcc -o $@ $^ -lpthread
.PHONY:clean
clean:
	rm -f http

all:
	gcc main.c -o httpw
	echo "Don't forget to look at DOC_ROOT in main.c and create the directory and an index.html file"

clean:
	rm httpw
NAME=performance-boggle
VERSION=latest

build:
	docker build -t $(NAME):$(VERSION) .

run:
	docker run --rm $(NAME):$(VERSION)

join:
	docker run --rm -it $(NAME):$(VERSION) /bin/bash

clean:
	docker rmi $(NAME):$(VERSION)

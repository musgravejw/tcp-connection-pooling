target:
		@mkdir -p dist
		@gcc src/connection-pool.c -o dist/server

clean:
	rm dist/*

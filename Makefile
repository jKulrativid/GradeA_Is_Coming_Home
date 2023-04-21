build_terrasort:
	docker compose build terrasort

build_powergrid:
	docker compose build powergrid

runtest:
	docker run --rm --env-file=.env -v C://Users//user//ProjeXt//HighPerfArch//finalproj//data//input:/input \
	-v C://Users//user//ProjeXt//HighPerfArch//finalproj//data//output:/output $(IMAGE) ./prog /input/$(INPUT) /output/$(OUTPUT)

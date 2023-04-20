build_terrasort:
	docker compose build terrasort

run_terrasort:
	docker compose up --build terrasort

runtest:
	docker run -v C://Users//user//ProjeXt//HighPerfArch//finalproj//data//input:/input \
	-v C://Users//user//ProjeXt//HighPerfArch//finalproj//data//output:/output $(IMAGE) ./prog /input/$(INPUT) /output/$(OUTPUT)

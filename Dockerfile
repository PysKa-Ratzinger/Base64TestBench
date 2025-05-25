FROM silkeh/clang

RUN apt update -y && apt install -y pkg-config googletest libgtest-dev


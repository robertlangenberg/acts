FROM docker:19.03.2
LABEL maintainer="Paul Gessinger"

RUN apk --no-cache add python3 git

COPY run.py run.py
ENTRYPOINT ["/run.py"]

FROM debian:bookworm-slim

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       build-essential make \
       openmpi-bin libopenmpi-dev \
       openssh-client openssh-server \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY src ./src
COPY include ./include
COPY Makefile .
RUN make

RUN mkdir -p /run/sshd /root/.ssh \
    && ssh-keygen -t rsa -b 2048 -f /root/.ssh/id_rsa -N "" \
    && cat /root/.ssh/id_rsa.pub >> /root/.ssh/authorized_keys \
    && chmod 700 /root/.ssh \
    && chmod 600 /root/.ssh/authorized_keys \
    && printf "Host *\n  StrictHostKeyChecking no\n  UserKnownHostsFile /dev/null\n" > /root/.ssh/config \
    && printf "Port 22\nPermitRootLogin yes\nPasswordAuthentication no\nPubkeyAuthentication yes\nUsePAM no\n" > /etc/ssh/sshd_config

COPY scripts/entrypoint.sh /app/entrypoint.sh
COPY scripts/run_cluster_job.sh /app/run_cluster_job.sh
RUN chmod +x /app/entrypoint.sh /app/run_cluster_job.sh

ENTRYPOINT ["/app/entrypoint.sh"]

# Base image with GCC and Python
FROM debian:trixie-slim

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && \
    apt-get install -y g++ python3 python3-pip && \
    pip3 install numpy

# Set working directory
WORKDIR /app

# Copy files
COPY . /app

# Command to build and run Makefile
RUN make

# Default command to run the linear regression
CMD ["make", "run"]


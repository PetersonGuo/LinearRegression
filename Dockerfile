# Base image with GCC and Python
FROM debian:trixie-slim

# Install dependencies
RUN apt-get update && \
    apt-get install -y g++ make python3 python3-pip python3-venv python3-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy project files
COPY . /app

# Create and configure the virtual environment
RUN python3 -m venv venv && chmod -R 755 venv

# Use the virtual environment as default Python
ENV PATH="venv/bin:$PATH"

# Install Python dependencies
RUN pip install -r requirements.txt --no-cache-dir

# Default command to run the project
#CMD ["make", "run"]


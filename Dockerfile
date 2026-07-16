# Stage 1: Build the C++ server
FROM alpine:3.24 AS builder

# Install build dependencies (g++ and make)
RUN apk add --no-cache g++ make

WORKDIR /app

# Copy source code and Makefile
COPY . .

# Build the server using the Makefile
RUN make clean && make

# Stage 2: Create the lightweight runtime image
FROM alpine:3.24

# Install runtime dependencies for C++ binaries
RUN apk add --no-cache libstdc++ libgcc

WORKDIR /app

# Copy the compiled binary from the builder stage
COPY --from=builder /app/server .

# Copy the static content folder
COPY --from=builder /app/content ./content

# Expose the default port
EXPOSE 3490

# Start the server
CMD ["./server", "content", "3490"]

# Use the official Cassandra base image
FROM cassandra:latest

# Optional: Copy custom configuration files if needed
# COPY cassandra.yaml /etc/cassandra/cassandra.yaml
# COPY cassandra-rackdc.properties /etc/cassandra/cassandra-rackdc.properties

# Optional: Copy initialization scripts to run on container startup
# These scripts will be executed by the entrypoint script of the base image
# COPY init-schema.cql /docker-entrypoint-initdb.d/init-schema.cql
# COPY init-data.cql /docker-entrypoint-initdb.d/init-data.cql

# Expose the default Cassandra CQL port (9042)
EXPOSE 9042

# Define the default command to run when the container starts
# The base image already handles this, but it can be overridden if necessary
# CMD ["cassandra", "-f"]
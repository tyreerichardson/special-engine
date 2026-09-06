# Local Cassandra (optional)

Open Docker Desktop first. From the application root, create the database once:

```sh
docker pull --platform linux/arm64 cassandra:5.0.9
docker run -d --name special-engine-cassandra --platform linux/arm64 \
  -p 127.0.0.1:9042:9042 \
  -v special-engine-cassandra-data:/var/lib/cassandra cassandra:5.0.9
```

These platform flags select native Apple Silicon/Linux ARM64; omit them on an x86
host so Docker selects its native image. If the named container already exists:

```sh
docker start special-engine-cassandra
```

Wait until this readiness query succeeds (first startup can take about a minute):

```sh
docker exec special-engine-cassandra cqlsh -e 'SELECT release_version FROM system.local;'
```

Apply schema and sample data once:

```sh
docker exec -i special-engine-cassandra cqlsh < cassandra/data.cql
```

Keyspace/table creation uses IF NOT EXISTS, but seed inserts generate new IDs and
timestamps. Reapplying the script can add more sample messages. The users table
starts empty. No separate host cqlsh installation or sudo is needed.

```sh
docker exec -it special-engine-cassandra cqlsh
docker logs --tail 50 special-engine-cassandra
docker stop --time 60 special-engine-cassandra
```

The named volume retains data when the container stops. The database and C++ driver
are separate dependencies; see the server README for driver installation. The
Dockerfile pins the same official version but does not automatically execute SQL
or CQL scripts during startup.

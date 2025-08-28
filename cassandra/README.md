# Cassandra Docker DB

## Starting Cassandra locally

To run this database you need to have docker installed
once you have docker installed you need to also have 'cqlsh' this is how you interact with cassandra within the terminal

while you are wihtin this folder you should run the command
***   docker build -t my-cassandra-image .   ***

then once this is completed you should have an image downloaded to your docker
and to run this image run this command 
***   docker run --name my-cassandra -p 9042:9042 -d my-cassandra-image   ***

and finally to connect to your locally running cassandra db you need to run this command
***   cqlsh localhost 9042   ***


## Intantiating the tables

To copy over the data to initialize the tables
run this script from this directory
***   sudo docker cp ./data.cql my-cassandra:/tmp/data.cql   ***

you should see a message if this was ran successfully

now you should be able to execute this script
***   sudo docker exec -it my-cassandra cqlsh -f /tmp/data.cql   ***
# netsDB - A UDF-Centric Analytics Database

This repository contains the code for netsDB, an analytics database that focuses on User-Defined Functions (UDFs). It is built on top of an earlier version of the open source project PlinyCompute. netsDB aims to optimize the serving of machine learning models by integrating them with data management systems, reducing management complexity and data transfer latency.

## Credits

netsDB is built on top of an earlier version of the open source project PlinyCompute. Visit the [PlinyCompute GitHub repository](https://github.com/riceplinygroup/plinycompute) for more information.

## Building

### Requirements

Software:
- clang/llvm/build-essential
- scons (http://scons.org/)
- bison
- flex
- LLVM/clang++3.8 or above
- Snappy (libsnappy1v5, libsnappy-dev)
- GSL (libgsl-dev)
- Boost (libboost-dev, libboost-program-options-dev, libboost-filesystem-dev, libboost-system-dev)

OS: Ubuntu-16, MacOS, Ubuntu-20

### Build Process

1. Install the required software dependencies:

   ```bash
   sudo apt install clang llvm build-essential scons bison flex libsnappy1v5 libsnappy-dev libgsl-dev libboost-dev libboost-program-options-dev libboost-filesystem-dev libboost-system-dev
   ```

2. Run the build command:

   ```bash
   scons
   ```

## Running netsDB on a Local Machine

To run netsDB on your local machine, follow these steps:

1. Start the pseudo-cluster:

   ```bash
   python scripts/startPseudoCluster.py #numThreads #sharedMemPoolSize (MB)
   ```

   Replace `#numThreads` with the desired number of threads and `#sharedMemPoolSize` with the desired shared memory pool size in megabytes.

2. To cleanup netsDB data on your local machine, run:

   ```bash
   scripts/cleanupNode.sh
   ```

## Running netsDB on a Cluster

To run netsDB on a cluster, follow the configuration and startup steps outlined below.

### Configuration

1. Download netsDB code from GitHub to the Master server and set `PDB_HOME` to the path of the GitHub repository. Edit `~/.bashrc` and add the following line:

   ```bash
   export PDB_HOME=/home/ubuntu/netsDB
   ```

   Replace `/home/ubuntu/netsDB` with the actual path to the GitHub repository.

2. Configure `PDB_INSTALL` to be the location where you want netsDB to be installed on the workers. Add the following line to `.bashrc`:

   ```bash
   export PDB_INSTALL=/disk1/netsDB
   ```

   Replace `/disk1/netsDB` with the desired installation directory path. Ensure that the user running the program has the necessary permissions in this directory.

3. After configuring `PDB_HOME` and `PDB_INSTALL`, run the following command to ensure the variables are set:

   ```bash
   source ~/.bashrc
   ```

4. Create a file named `serverlist` in the `$PDB_HOME/conf` directory and add the IP addresses of the Worker servers that should be part of the cluster. For example:

   ```
   10.134.96.184
   10.134.96.153
   ```

### Starting the Cluster

On the Master server, follow these steps to start the cluster:

1. Start the Master server:

   ```bash
   cd $PDB_HOME
   scripts/startMaster.sh $pem_file

.

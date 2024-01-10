# pulsar-client-cpp-perf

The **independent** performance tool for the Pulsar C++ client.

Though you can also build the tool in the [official repository](https://github.com/apache/pulsar-client-cpp/tree/main/perf), this project don't require you to build the Pulsar C++ client library from source. You can install the client library from the pre-built binanry first, see [here](https://pulsar.apache.org/docs/3.1.x/client-libraries-cpp-setup/) for more details.

## Requirements

- A C++ compiler that supports C++11, like GCC >= 4.8
- CMake >= 3.1
- Boost (with the `program_options` component)

## Build

First, clone this repo with all submodules.

```bash
git clone https://github.com/BewareMyPower/pulsar-client-cpp-perf.git
git submodule update --init --recursive
```

Then, build the tool with vcpkg.

```bash
cd pulsar-client-cpp-perf
# CMAKE_BUILD_TYPE is required for now, there is a bug in pulsar-client-cpp's port
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run `./build/perfProducer` for how to use it. For example, the following command run a producer against a Pulsar broker that listens on 6650 port locally with the following configs:
- Send 100000 messages per second, each message is 1 KiB by default.
- The batch size is 1000.
- The batch timeout is 1ms.

```bash
./build/perfProducer --service-url pulsar://localhost:6650 \
  --rate 100000 \
  --batch-size=1000 \
  --max-batch-publish-delay-in-ms=1 \
  my-topic
```

## Verify the version

Modify [vcpkg.json](./vcpkg.json) to test another version of `pulsar-client-cpp`.

> **Note**:
>
> Currently only 3.4.2 is available in vcpkg.

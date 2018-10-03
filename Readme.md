# Ball balancer

## build
```sh
$ mkdir build
$ cd build
$ cmake ..
$ make -j4

```

## Flash
```sh
$ openocd -f ../ocd-maple-stlink.cfg -c "init" -c "reset init" -c "flash write_image erase ballbalancer.bin 0x08000000" -c "reset" -c "exit"
```


# Space-Invader-LSO2018

Space-Invader game written in C, for getting to know the [ncurses](https://invisible-island.net/ncurses/) and [p-thread](https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html) library.

The final project in Operating System course at the 2nd year of Computer Science at the University of Cagliari, so to better undersand the mechanisms around an operating system ways to interrupt the code flow inside a software, both at low and high level of abstraction.

# Install

Clone repo then run commands:

```bash
sudo apt install -y libpthread-stubs0-dev libncurses5-dev build-essential
chmod +x space_invaders.sh
./space_invaders.sh
```

------



|          Commands          |                 Cheats                 |
| :------------------------: | :------------------------------------: |
| **Space** - Select / Shoot |        **P** - Increase Life +1        |
|         **W** - Up         |        **L** - Decrease Life -1        |
|        **A** - Left        |      **K** - Increase Missile +1       |
|        **S** - Down        |            **U** - Get EMP             |
|       **D** - Right        |           **J** - Get Shield           |
|      **Q** - Up Left       | **I** - Decrease (All) enemies life -1 |
|      **E** - Up Right      |                                        |
|    **F** - Activate EMP    |                                        |
|   **M** - Shoot Missile    |                                        |

# Example Video

![](/home/dshot/Desktop/example.gif)
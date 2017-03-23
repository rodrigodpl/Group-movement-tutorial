# Introduction

## The problem

When in a videogame a single unit must travel from point A to point B, we should only need a pathfinding algorithm (A\*, djikstra) properly coded to obtain a satisfactory result. That solution is useful, but limited. 

What would happen if we face a situation where a tile that in the moment the unit called the pathfinding method was walkable, but is not anymore where the unit is reaching it?

That could happen if a new building have just been placed in the map, or a troop has moved to a point in the path. The code only based in a pathfinding algorithm would let the unit travel across the colliding element as if it just wasn't there.

Now, let's imagine a situation where dozens of units are required to travel to a single point, a likely scenario in any RTS game. The result would be them overlapping one over the other to the point of appear a blurry mass.

That's only a brief example of game-breaking situations that could derive of not having proper code managing the displacement of units: enemies travelling over freshly built walls, confronted armies crossing each other without fighting[...]

You could say: "Yes, but collision module could take care of this"
```

OnCollision(...){
[...]
  if(unit->state == TRAVELLING){
     [...]
     App->pathfinding->CreatePath(Destination);
  }
```

That would work only for entities that can modify the walkability of the map, such as buildings. However, to have structures change the map settings is CPU-expensive and a bad coding habit. Also, it wouldn't work on units without an impressive CPU usage.

In order of providing a helping hand to developers in training, in the following lines it can be find a simple to use tutorial with code examples and exercises on the topic.

The goal of this tutorial is not only to explain how to solve the previous problems, but to provide a comprehensive solution for group movement and unit formations stable, CPU-cheap and Production-ready.


## Previous considerations

Before getting deep inside the topic, let's set some considerations so reader's code and the following explanation share mostly the same content. If the reader is using it's own solution, is not recommended to continue if we have skipped a step or could lead to unstable code.

### Waypoints

Waypoints are a optimization for plain A\* algorithm. Although it does not offer much better performance than a bit less memory consumption, they are going to be pretty useful later.

Waypoints appear every time in a path a new direction is taken. If the distance travelled is great and there aren't many obstacles, the number of waypoints will be quite less than the number of tiles of common paths.

By only registering the waypoints when creating a path we can obtain the final shape of the path, as there will always be straight lines connecting each waypoint with the next. The waypoint creation can be a small function called at the end of the pathfinder method, so units receive only the waypoints.

### High-level and low-level pathfinding

Although there are many games which don't differentiate between these two, it's a very effective approach and a balanced option between CPU usage and results quality. Let's see the main differences:

 - High-level: pathfinding in great scale. Needs to be quick and reliable. Will deal mostly with map navigation layer.
 - Low-level: pathfinding in a smaller scale. Needs to manage collisions with fluidity. Will deal mostly with colliders.

To provide kind of a reference, we could consider high-level every time the path to be taken is long enough to not fit inside the screen.

![Imgur](http://i.imgur.com/2uxUVeS.jpg)

### Other considerations

 - Tutorial will be set in a 2D environment.
 
_Although most solutions provided could be adapted to a 3D environment, let's keep it simple for the sake of understanding and generalization._
        
 - Units are going to feature two circular colliders around themselves. The hard collider, which will cover the unit's sprite space, and the soft collider, which will cover an area of "personal space" around the unit.
 
_Why circles? They offer a smooth experience when used as collider, as well as not very difficult collision calculation methods. Why hard and soft colliders? For priority managing of the collisions, which will be seen later._

 - Collision module is going to detect collisions one frame before happening.
 
_This means, the collider module update should take in consideration not the current position but where the unit is going to be after applying it's current speed. That allows us to prevent collision before happening and avoid game breaking situations._

## Base pathfinding code

So, after introducing the previous considerations, our unit state machine when moving should be something like:
```
[...]
case WAYPOINT_REACHED:
     if(ispatrolling){
        current_dest = patrol_route->NextWaypoint(); 
        state = WAITING_FOR_PATH;
    } 
    else{
        if(Path->IsEmpty())
            state = DEST_REACHED;
        else
            current_dest = Path->Pop();
    }
case WAITING_FOR_PATH:                                                       // we are patrolling or an order have been issued
    Path = App->pathfinding->CreatePath(current_dest);
    if(Path == NULL)                                                   // if a path can't be found, stop
        state = DEST_REACHED;
    else
        state = MOVING;
case MOVING:
    direction = GetDirection(current_dest);                  // check the direction the path follows
    iscolliding = CheckCollisions(position + (direction * speed));    // check for collisions in our next step
    if(iscolliding)                                                // if we collide, stop;
       state = DEST_REACHED;
    else
       [...] (Update position, animation, Path...)
case DEST_REACHED:
    StopMoving();
 
}    
```

This is far from being a solid solution, but is intuitive and simple enough so it can be understood easily the changes we are going to apply. Right now, our units just stop when they find something in their way. Obviously, this is not what we want them to do, but for now let's just keep it like that for simplicity.

# Managing collisions

## Position and collision prediction

One of the first problems that the previous piece of code is the fact that, although when checking collisions were are using the position of (t+1) of our unit, we are checking against colliders in (t). This means, a obstacle that is not anymore there when we reach it would make us stop anyway.

If we want to manage huge groups of units moving at the same time through the same area we'll need to be more than just one step ahead so units can recalculate their paths accordingly. In order to do so, we are going to create a queue of predicted positions. To obtain them, we just run our movement method several times without updating unit's current position.

This may seem a very CPU-expensive solution, specially when starting a fresh new path, but these records will be kept to be reused further on. Also, as we have already calculated the unit's future positions, we don't need to recalculate them, just update using the records.

This queue will be coded as a rolling list, this means, once the first call is made and the queue is filled with a certain number of predicted positions, on each update we will just pop to update the current position and pushback the next step of our last predicted position.

Each node of the queue should contain the position and direction of the troop in that moment. Also, it should have an assigned collider so we can predict collisions in future updates.

![Imgur](http://i.imgur.com/CehsCMI.jpg)

### Fixed time step and interpolation

If we need to simulate a series of positions in one single call, we are going to need fixed time step for those calculations. However, even by carefully choosing the interval this could lead to code issues further on.

A way to avoid this is to create an interpolation system that adjusts the series by interpolating with the real current frame time. As we have direction and fixed time interval values, this shouldn't be hard.

The interpolation system can be used to avoid as well the situation where a unit "jumps" over a obstacle because the update has moved the unit to the other side in a single frame without triggering the collider. By interpolating enough positions we can cover the whole space between two updates so there's always a collider in the path.

![Imgur](http://i.imgur.com/A7eF2ut.jpg)

## Coordinating units

With the new predicting system implemented, the pathfinding algorithm can be recoded so the unit circles around collisions rather than just stopping. Let's put a practical scenario to be more visual.

We are unit A, travelling to East Fort, when we find a cluster of allied troops heading to West Fort. We don't want to interact with them, just surround them and continue in our way. How do we do?

Our prediction system will be checking collisions in the last added prediction, so we will notice the collision a few frames before happening. Our goal now is to change the path of unit A so it avoids the coming cluster of units.

Here is were high-level and low-level pathfinding start to differentiate. Our previous path, the one obtained by calling the pathfinding method, is our "goal path", as it's the shortest path to destination. Therefore, our new path will avoid the collision keeping as near as possible of the original route, and then linking with it again. 

As this method could be called hundreds of times in few seconds, the classic pathfinding method would be too slow, and most times unnecessary as the change in the path could be of just one tile. Let's check instead this function:
```
fPoint GetNearestAdj(){
    iPoint nearest_adj = {-1,-1};
    for(int i = -1; i < 2; i++){
        for(int j = 0; j < 2; j++){
            if(DistManhattan((current_tile + {i,j}), dest_tile) < Dist_Manhattan((current_tile + adj_tile), dest_tile)){
                direction = GetDirection(current_tile + {i,j});                 
                if(CheckCollisions(position + (direction * speed) == NULL)
                   nearest_adj = {i,j];
            } 
        }   
    }
    return GetDirection(current_tile + {i,j});
}
```
This is a kind of "pathfinding in miniature". It looks for the nearest tile to their next waypoint that will not trigger a collision, including the current one, and returns the position the unit would be if it moved in the tile direction. This position should be stored in the unit's predicted positions.

![Imgur](http://i.imgur.com/OegeqAS.jpg)

As precisely predicted positions define how unit position is going to be updated, by calling this method when a collision is found we avoid it right then, while our unit keeps moving in almost the same direction towards it's next waypoint. If in the next update the unit keeps finding an obstacle, it will try to circle it as well and so on.

![Imgur](http://i.imgur.com/LauOAtj.jpg)

This function also manages that units don't move away from the original route, as it will always be chosen the nearest tile to the path. The nearest tile can be the current one depending on the obstacle's nature, making the unit wait for a certain collider to move away.

### "Pushing"

We have been describing the previous situation as Unit A moving away from it's path so it does not collide. However, if instead of a moving cluster we had idle troops, it would more time efficient for Unit A that idle troops moved away to let it pass. This action will be called "pushing" the idle unit from now on.

The previous function can be used as well for this: Each idle unit should begin a path with it's current position as single waypoint. The method will try to keep the unit on it's position, but as it's colliding, it will then choose any empty tile around it to move away and let Unit A pass. As the Unit A has left, the collision disappears and the idle unit goes back to it's position.

However, pushing units has limitations. A unit will only let be pushed if there at least one empty space around it. If it's surrounded by other colliders or unwalkable tiles, let's say a surrounding circle of troops, this method could not work as expected.

### Hard Push

Let's imagine a situation on where our map features a narrow canyon where there's only space for a unit to move left or right. Units A, B and C are stationed in the canyon, serving as guards, forming a line. Our hero H wants to cross the canyon, and the only way to do so would be if C moves to the other end and lets B and A get out of the way.

However, the first collision would be with A, which can't be pushed away as it has B right behind. This will just stop H and it would never reach the other side. To fix this situations, we must code a "Hard Push" method.

![Imgur](http://i.imgur.com/Hs0Rk2Z.jpg)

Hard push works more like pathfinding than like pushing. The idea is to figure out and store which are the minimum set of colindant units which must move so another unit can travel across them.  

```
bool HardPush(fPoint direction){
    Collider* c1 = NULL; 
    bool ret = false;
    list<fPoint> dir_list;
    dir_list.add({direction.y, - direction.x});      // to left
    dir_list.add({direction.x, - direction.y});      // to right
    dir_list.add({- direction.y, - direction.x});    // behind
    
    for(list_item* item = dir_lisr.start; item; item = item->next){
       if(c1 = CheckCollisions(position + (item->data * speed)){
           if(c1->IsUnit)
               ret = c1->HardPush(item->data);
       }
       else
            ret = true;
       
       if(ret){
           Waypoints.Pushback(position + (item->data * speed));  
           Waypoints.Pushback(position);
           break;
       }
    }
    dir_list.Clear();
    return ret;
}
```

Hard push is significantly more complex than basic pushing. The first notorious change would be that is a recursive function, as it needs to keep pushing units of a possible cluster away until one of them finds an empty space. 

It does also work with waypoints instead of pred.pos. This happens because a hard-pushing movement can require very close coordination of troops, which if managed through pred.pos. would need a lot of recalculations on each update. By stacking waypoints we just record the moves a troop has done to let others pass, so they are travelled inversely when finished.

NEEDS OPTIMIZATION (SHORTEST PATH, DIAGONAL MOVEMENT)

## Priorities and collision solving

Lets return to the example where Unit A and the cluster are moving. What would happen if some troop of the cluster, let's call it unit B, tried to avoid unit A simultaneously? It could result on it, for example, turning right to let it pass, so that would make units C, D, and E, which are right behind B, to predict a collision between them.

That would create a chain effect where all units will behave crazy trying to dodge each other and performing tons of unnecessary computations. To avoid this, we must use unit prioritizing and collisions states. With those tools implemented, once a unit has solved a collision, nearby units will ignore it.

This means, units will manage collisions in base of the state and priority of each one. Priority is given by the coder and could be derived from any mechanism (size, power, speed...).

Then, we can group the possibles state values in:

 - Unit A is moving / Unit B is idle:
     1. Higher priority unit pushes lower.
     2. If it can't be done, higher priority unit tries to avoid lower.
     3. If it can't be done, higher priority unit hard pushes lower.
     4. If none can be done, stop.
     
  - Unit A is moving / Unit B is moving:
     1. Lower priority unit tries to avoid higher.
     2. If it can't be done, higher tries to avoid lower.
     3. If it can't be done, lower stops, higher pushes it.
     4. If it can't be done, higher hard pushes it.
     5. If none can be done, both stop.
      
This flowchart guarantees that all possible solutions are tried before cancelling the move, ordered so to keep the unit travel time short and CPU usage low.

We will classify oncoming collisions as:

 - Unsolved: the collision have been noticed, so the code computes the logic decisions to manage it.
 - Solving: the pertinent code methods have been called and the corresponding unit is in process to avoid the collision.
 - Solved: the collision has not happened. The corresponding unit return to it's previous position.

This classification will appear in the code as flags to trigger unit movements and avoid two units solving the same the collision.

# Group movement

Up to now, although we have used several units in our examples, the code was treating it as unit-to-unit collisions. This means, as if every unit had been assigned an order individually. However, the most usual way to manage units in RTS games is in groups of certain units, which are expected to act cohesively and coordinated.

To manage this higher step of complexity, the first we need to do is to code a Group class. This class will override unit controls and manage positions, orientations and collisions. But before, let's keep in mind some considerations.

### Group characteristics

 - Units inside a group will only collide against themselves with hard colliders. Soft collider calls will be ignored.
 
_If troops are forming tightly, we should receive hundreds of calls in few seconds just because the group is turning or any other controlled action. Group class manages this, so collision module shouldn't do anything._
 
  - Units inside a group move at the same speed, take (almost) the same path and arrive at the same time.
  
_If we not follow those rules, the purpose of a group (form a solid block of units) is lost, as units just scramble themselves around the map taking each it's own way._

 - All groups will feature a commander, which will act as the "representative" of the group.
 
_The immediate function of the commander is to call only once functions whose result can be extrapolated to whole group, as pathfinding. However, later we will see more uses for this element._
 
 - A group can hold, but not necessarily, a formation. Grouped formations coordinate with High-level pathfinding (Waypoints), while formations use Low-level (pred.positions).
 
_When grouped, units will only try to move close to each other. When formed, units must maintain certain positions inside the group. Formations will be reviewed later on this article._
 
## Group class

```
class Group{
public:
    int type;
    int state;
    fPoint center;
    float max_speed;
    Unit* commander;
    int num_of_units;
    list<fPoint> unit_positions;
    list<fPoint> expected_positions;

public:
    void Update();
    [...]
}
```
POSITONS SHOULD WAYPOINTS

To manage it's units, Group class uses the commander as reference. It will only trace the commander's movement and order the remaining units to adjust their path to try to get closer to him. This is performed via waypoints and speed adjustment. 

 - Waypoints:
 
Once a group have been selected and asked to move somewhere, the commander will execute pathfinding method with the specifications of the biggest unit in the group. Then, on each waypoint of it's path, it will locate waypoints for each unit on the nearest possible tile of the their own waypoint. 

This system could rise some kind of odd behaviour, yet practical, if a commander waypoint is surrounded by several unavoidable colliders. This would cause unit's waypoints to be created quite far from the commander's. However, if we put this situation in perspective, these units will need to travel through that narrow space to reach the next waypoint, so having them properly spaced from each other will increase the fluidity of the movement. 

![Imgur](http://i.imgur.com/XdPMCAr.jpg)

To keep collisions at a minimum, the method looking for available tiles for troops waypoints should always check tiles in the same order, as well as when assigning them to troops. 

 - Speed adjustments:
 
Waypoints are only half of the system, though. Now, our units would follow similar paths, but cavalry would reach the goal much faster than siege units, which are quite slower. A way to solve this issue is to adjust some units speed on the go. This adjustment shouldn't be applied directly to the unit base speed but multiplying the value for some fixed intervals.

The commander will work as reference for this adjustments. Although more compex calculations can be developed, a simple and effective way to manage this system is:
 
 1. If a unit has less waypoints left in it's path than the commander, it should slow down based in the distance from it's current waypoint and commander's.
 2. If a unit has the same waypoints left than commander's, but it's nearer to the current waypoint than commander, it should slow down based in the distance between commander and waypoint and unit and waypoint.
 3. If a unit has the same waypoints left than commander's and more or less the same distance to waypoint, it moves at group speed.
 4. [...] / 5. [...] 
 
4 and 5 would be the same as 2 and 1 respectively, but inversed so units move faster when farther from waypoint.

This system should have implemented caps in max/min speed multiplier value to avoid catapults "sprinting" to reach the group, as well as some margin before triggering speed modifiers to avoid new velocities being calculated on every frame to solve slight deviations. 

## Formations

Formations represent units trying to maintain a specific position in relation to the other units in the group through all path. Although formations are built inside group environment as a state of it, they behave differently. In a group, units try to keep close to the commander creating kind of a cluster. In a formation, units must in a certain position at a certain time.

It's obvious that the level of precision required increases dramatically if formations are to be kept through a path. Thus, the High-level pathfinding system used for groups is to simple for formations. Instead, we are going to code a similar method which works with predicted positions.

### Building the formation

Our first step before start travelling towards our destination is to build the formation. If a formation can move without being formed, it does not have any use at all. In order to minimize collisions and time spent in this step, we are going to use a central point approach.

In order to do so, first we calculate the center of the smallest quadrilateral containing all group units current position. If that center is near colliders, we will displace it as less as possible until there's enough space. Then, we need a sorting algorithm to order units in function to their proximity to the center.

The nearest units will position in the nearest available space of the center they can fit on, taking in account the formation specifications. The commander will take the central position. If there are several units that could be chosen as the commander, we will select the nearest to the center.

![Imgur](http://i.imgur.com/ofoXxjJ.jpg)

### Moving the formation

Once the formation has been built, the next step is update positions so units start advancing. First, we call pathfinder from commander and stablish a route for him to follow. The pathfinding will be called keeping in mind the whole formation size. Then commander will perform the first movement towards the first waypoint.

To keep the formation advancing, we must replicate the same movement in every unit on the formation. All of this will be recorded in each unit predicted positions queue. The result is the whole formation moving at the same time, without losing formation nor colliding between units.

We must mind, however, that formations have directions. If we are forming a vertical line moving from left to right, and turn to face upwards, we expect the whole formation to turn. If not, what before was a line now is a column. The simplest way to achieve this is to stop and build formation again after every waypoint.

However, that solution could be very slow in game time, as well as cause unpredicted behaviour when used in narrow spaces. We encourage the reader to come up with a system fitting for their game and code it themselves.

# Exercises 

Next, we will see some simple exercises to start putting the basics of this tutorial in practice. As we have covered lots of information and possible implementations, we will reduce this exercises in creating a base to start building your own code over it.

First, let's see some differences between high-level pathfinding and low-level. In order to do so, we are going to develop some simple methods to avoid unit overlapping.

To do the exercises, download the code attached in this page and use some code tool like Visual Studio. 

Executable controls:
 - Esc: Quit
 - arrow keys: move camera
 - left_click: select
 - drag: multiselect
 - right_click : move
 - F1: debug


### TODO 1

First, we need to start checking not only the walkability of the map but if there's any unit blocking our path.
Code the function IsOcuppied() to check if there are any colliders in our way.

### TODO 2

Once we have found a tile we cannot go through, we need to find some other way to reach our destination. Calling again the pathfinder would be very slow, so let's use a quick function the find the nearest available tile. The function is half done, you just have to finish it.

### TODO 3 

Squared colliders are more commonly used when starting to code, but circular colliders can provide better functionality. Let's figure out the equation to know if two circular colliders intersect. 

Use F1 on the executable to know if a collider is being called.

### TODO 4

Use your freshly finished FindNearestAvailable() to provide an available tile for the colliding unit. Remember to pop first the front node if the path is not empty

### Github link

[Github repository](https://github.com/rodrigodpl/Group-movement-tutorial)

# Useful links

[gamasutra article](www.gamasutra.com/view/feature/131720/coordinated_unit_movement.php)

[document on formations](http://sander.landofsand.com/publications/CIG08Heijden.pdf)

[explanation and benchmark for group movement](http://apexgametools.com/learn/apex-utility-ai-documentation/real-time-strategy-demo/apex-utility-ai-rts-demo-06-group-movement/)

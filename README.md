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

### Other considerations

 - Tutorial will be set in a 2D environment.
 
_Although most solutions provided could be adapted to a 3D environment, let's keep it simple for the sake of understanding and generalization._
        
 - Units are going to feature two circular colliders around themselves. The hard collider, which will cover the unit's sprite space, and the soft collider, which will cover an area of "personal space" around the unit.
 
_Why circles? They offer a smooth experience when used as collider, as well as not very difficult collision calculation methods. Why hard and soft colliders? For priority managing of the collisions, which will be seen later._

 - Collision module is going to detect collisions one frame before happening.
 
_This means, the collider module update should take in consideration not the current position but where the unit is going to be after applying it's current speed. That allows us to prevent collision before happening and avoid game breaking situations._

### Base pathfinding code

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

## Position and collision prediction

One of the first problems that the previous piece of code is the fact that, although when checking collisions were are using the position of (t+1) of our unit, we are checking against colliders in (t). This means, a obstacle that is not anymore there when we reach it would make us stop anyway.

If we want to manage huge groups of units moving at the same time through the same area we'll need to be more than just one step ahead so units can recalculate their paths accordingly. In order to do so, we are going to create a queue of predicted positions. To obtain them, we just run our movement method several times without updating unit's current position.

This may seem a very CPU-expensive solution, specially when starting a fresh new path, but these records will be kept to be reused further on. Also, as we have already calculated the unit's future positions, we don't need to recalculate them, just update using the records.

This queue will be coded as a rolling list, this means, once the first call is made and the queue is filled with a certain number of predicted positions, on each update we will just pop to update the current position and pushback the next step of our last predicted position.

Each node of the queue should contain the position and direction of the troop in that moment. Also, it should have an assigned collider so we can predict collisions in future updates.

### Fixed time step and interpolation

If we need to simulate a series of positions in one single call, we are going to need fixed time step for those calculations. However, even by carefully choosing the interval this could lead to code issues further on.

A way to avoid this is to create an interpolation system that adjusts the series by interpolating with the real current frame time. As we have direction and fixed time interval values, this shouldn't be hard.

The interpolation system can be used to avoid as well the situation where a unit "jumps" over a obstacle because the update has moved the unit to the other side in a single frame without triggering the collider. By interpolating enough positions we can cover the whole space between two updates so there's always a collider in the path.

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

As precisely predicted positions define how unit position is going to be updated, by calling this method when a collision is found we avoid it right then, while our unit keeps moving in almost the same direction towards it's next waypoint. If in the next update the unit keeps finding an obstacle, it will try to circle it as well and so on.

This function also manages that units don't move away from the original route, as it will always be chosen the nearest tile to the path. The nearest tile can be the current one depending on the obstacle's nature, making the unit wait for a certain collider to move away.


## Priorities and collision solving

The previous implementation in the code seem effective, but what would happen if some troop of the cluster, let's call it unit B, tried to avoid unit A simultaneously? It could result on it, for example, turning right to let it pass, so that would make units C, D, and E, which are right behind B, to predict a collision between them.

That would create a chain effect where all units will behave crazy trying to dodge each other and performing tons of unnecessary computations. To avoid this, we must use unit prioritizing.

This means, units will manage collisions in base of the state and priority of each one. Priority is given by the coder and could be derived from any mechanism (size, power, speed...).

We will classify oncoming collisions as:

 - Unsolved: the collision have been noticed, so the code computes the logic decisions to manage it.
 - Solving: the pertinent code methods have been called and the corresponding unit is in process to avoid the collision.
 - Solved: the collision has not happened. The corresponding unit return to it's previous position.

This classification will appear in the code as flags to trigger unit movements and avoid two units solving the same the collision.

### PushUnit()

The adapted A\* algortithm we have elaborated is quite resilient, but very resource-expensive. In the example of the unit and the cluster, even by having the code work properly and obtain the expected result, we would have called the pathfinder method dozens of times.

We must keep in mind that up to now we have just reviewed units acting individually, not as a group, which has some specific features that we will see later. Thus, is not so likely to have that many units colliding if each one of them have been given the order one at a time.

Therefore, although we keep needing the pathfinder in some cases, for most situations we can use a simpler and cheaper method: PushUnit(). This recursive method will only be called if one of the units is idle when colliding.


```
PushUnit(Unit* unit_pushed, int start_frame, int end_frame){
    bool ret = false; 
    Collider* c2 = NULL;
    if(c2 = CheckCollisionOnFrame(unit_pushed->position.y + this->collider->radius, unit_pushed->position.x), start_frame){   // "this" is the pushing unit 
        if(c2->IsUnit())
            unit_pushed->PushUnit(c2->GetUnit(), start_frame, end_frame);
        else
            return ret;
    }   
    unit_pushed->predicted_positions.InsertAt(start_frame - current_frame, Pred_Pos(unit_pushed->position.y + this->collider->radius, unit_pushed->position.x)
}
```
Then, we can group the possibles state values in:

 - Unit A is moving / Unit B is idle:
    -

### Markdown

Markdown is a lightweight and easy-to-use syntax for styling your writing. It includes conventions for

```markdown
Syntax highlighted code block

# Header 1
## Header 2
### Header 3

- Bulleted
- List

1. Numbered
2. List

**Bold** and _Italic_ and `Code` text

[Link](url) and ![Image](src)
```

For more details see [GitHub Flavored Markdown](https://guides.github.com/features/mastering-markdown/).

### Jekyll Themes

Your Pages site will use the layout and styles from the Jekyll theme you have selected in your [repository settings](https://github.com/rodrigodpl/Group-movement-tutorial/settings). The name of this theme is saved in the Jekyll `_config.yml` configuration file.

### Support or Contact

Having trouble with Pages? Check out our [documentation](https://help.github.com/categories/github-pages-basics/) or [contact support](https://github.com/contact) and we’ll help you sort it out.

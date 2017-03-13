## The problem

When in a videogame a single unit must travel from point A to point B, we should only need a pathfinding algorithm (A\*, djikstra) properly coded to obtain a satisfactory result. That solution is useful, but limited. 

What would happen if we face a situation where a tile that in the moment the unit called the pathfinding method was walkable, but is not anymore where the unit is reaching it?

That could happen if a new building have just been placed in the map, or a troop has moved to a point in the path. The code only based in a pathfinding algorithm would let the unit travel across the colliding element as if it just wasn't there.

Now, let's imagine a situation where dozens of units are required to travel to a single point, a likely scenario in any RTS game. The result would be them overlapping one over the other to the point of appear a blurry mass.

You could say: "Yes, but collision module could take care of this"
```

OnCollision(...){
[...]
  if(unit->state == TRAVELLING){
     [...]
     App->pathfinding->CreatePath(Destination);
  }
```

That would work only for entities that can modify the walkability of the map, such as buildings. However, to have structures change the map settings is CPU-expensive and a bad coding habit. Also, it wouldn't work on units, or if the time between frames rises too much.

Why? Most of times code takes in account variable time rate when updating units positions, this means, it makes the distance travelled calculations based on the time since last update rather than a fixed interval on each update. 

If a game item collider is small and the frame rate drops too much, the displacement of the unit will be greater than the colliding item size and the unit will "teleport" to the other side.

IMAGE

That's only a brief example of game-breaking situations that could derive of not having proper code managing the displacement of units: enemies travelling over freshly built walls, confronted armies crossing each other without fighting[...]

In order of providing a little light over the matter, in the following lines it can be find a simple to use tutorial with code examples and exercises on the topic.

The goal of this tutorial is not only to explain how to solve the previous problems, but to provide a comprehensive solution for group movement and unit formations stable, CPU-cheap and Production-ready.


## Previous considerations

Before getting deep inside the topic, let's set some considerations so reader's code and the following explanation are composed of more or less the same content. The following rules are not only suggestions, as they will have impact further on, so we recommend the reader to not continue until these considerations have been written in his code or at least have them properly understood.

 - The scenario is going to be a isometric 2D environment.
 
_Although most solutions provided here would work in a 3D environment, let's keep it simple for the sake of understanding and generalization._
        
 - Units are going to feature two circular colliders around themselves. The hard collider, which will cover the unit's sprite space, and the soft collider, which will cover an area of "personal space" around the unit.
 
_Why circles? They offer a smooth experience when used as collider, as well as not very difficult collision calculation methods. Why hard and soft colliders? For priority managing of the collisions, which will be seen later._

 - Collision module is going to detect collisions one frame before happening.
 
_This means, the collider module update should take in consideration not the current position but where the unit is going to be after applying it's current speed. That allows us to prevent collision before happening and avoid game breaking situations._

So, after introducing the previous considerations, our unit state machine when moving should be something like:
```
[...]
case WAITING_FOR_PATH:                                                       // we are patrolling or an order have been issued
    Path = App->pathfinding->CreatePath(current_dest);
    if(Path == NULL)                                                   // if a path can't be found, stop
        state = DEST_REACHED;
    else
        state = MOVING;
case MOVING:
        direction = GetDirection();                  // check the direction the path follows
        iscolliding = CheckCollisions(position + (direction * speed));    // check for collisions in our next step
        if(iscolliding)                                                // if we collide, stop;
           state = DEST_REACHED;
        else
           [...] (Update position, animation, Path...)
case DEST_REACHED:
    StopMoving();
    if(ispatrolling){
        current_dest = patrol_route->NextWaypoint(); 
        state = WAITING_FOR_PATH;
    } 
    else
        state = IDLE;
}    
```

This is far from being a solid solution, but is intuitive and simple enough so it can be understood easily the changes we are going to apply. Right now, our units just stop when they find something in their way. Obviously, this is not what we want them to do, but for now let's just keep it like that for simplicity.

## Position and collision prediction

If we want to manage huge groups of units moving at the same time through the same area we'll need to be more than just one step ahead so units can recalculate their paths accordingly. In order to do so, we are going to create a queue of predicted positions. To obtain them, we just run our movement method several times without updating the position at the end.

This may seem a very CPU-expensive solution, specially when starting a fresh new path, but these records will be kept to be reused further on. Also, we have already calculated the unit's future positions, so we can just update the unit's position using them.

This queue will be coded as a rolling list, this means, once the first call is made and the queue is filled with a certain number of predicted positions, on each update we will just pop to update the current position and pushback the next step of our last predicted position.

Each node of the queue should contain the position and direction of the troop in that moment. Also, it should have an assigned collider so we can predict collisions in future updates.

# Fixed time step and interpolation

If we need to simulate a series of positions in one single call, we are going to need fixed time step for those calculations. However, even by carefully choosing the interval this could lead to problems in the future.

A way to avoid this is to create an interpolation system that adjusts the series by interpolating with the real current frame time. As we have direction and fixed time interval values, this shouldn't be hard.

The interpolation system can be used to avoid as well the situation where a unit "jumps" over a obstacle because the update has moved the unit to the other side in a single frame without triggering the collider. By interpolating enough positions we can cover the whole space between two updates so there's always a collider in the path.

## Coordinating units








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

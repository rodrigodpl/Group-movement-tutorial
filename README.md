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

The goal of this tutorial is not only to explain how to solve the previous problems, but to provide a comprehensive solution for group movement and unit formations stable and CPU-cheap.

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

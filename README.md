# Group Movement Tutorial

When in a videogame a single unit must travel from point A to point B, we should only need a pathfinding algorithm (A\*, djikstra) properly coded to obtain a satisfactory result. That solution is useful, but limited. 

What would happen if we face a situation where a tile that in the moment the unit called the pathfinding method was walkable, but is not anymore where the unit is reaching it?

That could happen if a new building have just been placed in the map, or a troop has moved to a point in the path. The code as it is would let the unit travel across the colliding element as if it just wasn't there.

You can use the [editor on GitHub](https://github.com/rodrigodpl/Group-movement-tutorial/edit/master/README.md) to maintain and preview the content for your website in Markdown files.

Whenever you commit to this repository, GitHub Pages will run [Jekyll](https://jekyllrb.com/) to rebuild the pages in your site, from the content in your Markdown files.

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

# Assignment 1 - Gwent

## Steps completed

### 1. Setup
- The `Tema1` scene follows the structure from the labs (lab1-lab5)
- Orthographic camera over a fixed logical space of 1280×720 (origin bottom-left), independent of the window size
- `transform2D.h` and `object2D` copied from lab3, in the `tema1::` namespace (avoids link conflicts with lab3)
- Shapes: square, circle, ring, triangle, diamond, star, cross
  - all white, size 1, centered at the origin
  - size/position/rotation come from the model matrix, color from the `Color` shader
- Depth test disabled: draw order gives the layering (what is drawn last is on top)

### 2. Game board
- Zones: enemy hand (top), board split into two halves, player hand (bottom), decks (right), lives (left)
- Cards colored by type (Damage red, Heal green, Shield blue, etc.), with a type-specific geometric symbol
- Temporary cards have a small diamond in the corner
- Value V shown as V small red squares
- Enemy cards drawn face down (blue rectangle with an X)
- Life: one green square per unit (gray for lost ones); shield on a separate blue row

### 3. Retina display fix
- The framework's `window->GetResolution()` applies the scale factor twice on HiDPI screens
- The viewport uses `glfwGetFramebufferSize` (real pixels)
- Mouse coordinates are converted with `glfwGetWindowSize` (points, same space as the mouse)

### 4. OOP structure
- `Rect`: rectangle + hit test (`Contains`)
- `Card`: type, value, position, look (color, symbol), animation
- `Player`: deck, hand, board row, life, shield, drawing zones
- `Tema1`: the scene (camera, game flow, drawing, input)

### 5. Drag & Drop
- Left click on a hand card selects it (test `x_min <= x <= x_max`, `y_min <= y <= y_max`)
- The dragged card follows the cursor and is drawn on top of everything
- Released on the player's own board half: the card moves to the board
- Released anywhere else: the card goes back to the hand
- Translation animation: each frame, every card covers a fraction of the remaining distance to its `target`
  - the same rule handles returning to the hand, rearranging the hand and arranging the board
- `LayoutRow` places cards on a centered row, keeping their order

### 6. Drawing cards from the deck
- Each player has a deck of 20 random cards
- At the start, 10 cards are dealt to each player, one every 0.15s, alternating
- The card appears on the deck and slides to its slot in the hand
- The `R` key starts a new game

### 7. Enemy
- Turn phases: `Dealing` → `PlayerTurn` → `EnemyTurn` → `Reveal` → `PlayerTurn` ...
- After the player places a card, the enemy picks a random card from its hand and puts it on its board
- The card arrives face down, then flips
- Flip = rotation around the vertical axis: scaling on X by `|cos(flip·π)|`
  - the back is visible in the first half, the front in the second
- Card details are drawn relative to the card's center, then the card's matrix places them (hierarchical transforms)

## To do
- Card effects, applied in order: Scorch - Spy - Clear Weather - Frost Weather - War Horn - Shield - Damage - Heal
- End of the game

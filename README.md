ImCard - simple auto-expandable card-view implementation with Dear ImGui
=====

### Requirements:
 - std 20+
 - Dear-ImGui 1.88+

### Features:
 - No objects (classes/structs), only inline direct call.
 - Auto-expandable group/background.
 - Changable color & rounding scale.
 - Card style customization.

### Usage:
**Use BeginCard()/EndCard() functions for group your widgets in cart-view**

Code:
```
BeginCard("My Card View");
  // your widgets
EndCard();
```

### How to change background and rounding scale?
Code:
```
ImColor color = ImColor(255, 255, 255, 255);
float rounding = 15.0f;
---
BeginCard("My Card View", color, rounding);
  // your widgets
EndCard();
```

### Read functions summary for get more info!

### Auto-Expanding preview
Taken in an [example-project](https://github.com/VfxFly/ImCard/tree/3d702221402bd0c81272633b2574c289524cf913/Example)
<br>![controls_example](https://github.com/VfxFly/ImCard/blob/8bf697b5183b83dbc3bd38926722a7dc100cc03f/Images/ImCards.gif)

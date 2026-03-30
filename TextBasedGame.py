# Kain Semonis
current_room = 'Bedroom'  # Sets starting room as the Bedroom.
previous_room = None    # Gives a room to revert to if the player walks into a wall.
direction=input("Press enter to wake up.")
counter = 0 # Counts how many times the player walks into a wall.
collection_counter = 0


# Defines the rooms and their connections, as well as adding 'walls' for the player to get amnesia from.
rooms = {
        'Bedroom': {'Item': ' ','description': "You're in a bedroom. You're pretty sure that it's yours, but can't be certain.\n To the east you see a hallway, and to the north you see a bathroom.",'East': 'Hallway', 'North': 'Bathroom','South': 'Wall','West': 'Wall'},
        'Hallway': {'Item': 'left shoe', 'description': "You've entered the hallway. The ceiling is too high on one side, and too low on the other. You can't decide if the middle is too high or too low.\n You can smell the kitchen to the north, hear the sounds of outside to the east, see another bathroom to the south, and feel the bedroom pulling you west.",'North': 'Kitchen', 'East': 'Outside', 'South': 'Bathroomier','West': 'Bedroom'},
        'Kitchen': {'Item': 'pair of socks', 'description': "You're in the kitchen. You wish you weren't.\n To the east you see the living room, to the south you see the hallway.",'North': 'Wall', 'East': 'Living Room', 'South': 'Hallway', 'West': 'Wall'},
        'Living Room': {'Item': 'pair of pants', 'description': "You're in the living room. You somehow know that the TV doesn't work, and that it might be related to not being plugged in, but you're not a high enough level wizard to be sure.\n To the west, you can still smell the kitchen.",'North': 'Wall', 'East': 'Wall','South': 'Wall', 'West': 'Kitchen'},
        'Bathroomier': {'Item': 'set of keys', 'description': "Somehow you know that this isn't just the bathroom, it's the bathroomier. This one actually has working plumbing, and marble tile floors.\n To the north, you see the hallway. To the east, you see the door to the late 19th century Victorian Study.", 'North': 'Hallway', 'East': 'Victorian Study', 'South': 'Wall', 'West': 'Wall'},
        'Bathroom': {'Item': 'shirt', 'description': "You're in the bathroom. It has carpeted floors.\n To the south, you can feel your bedroom pulling you.", 'North': 'Wall', 'East': 'Wall', 'South': 'Bedroom', 'West': 'Wall'},
        'Victorian Study': {'Item': 'right shoe', 'description': "You've entered Earl Stephen Chandler IV's study. It smells of old wood and ink. Stephen himself is sitting at his desk, bent over a piece of paper and writing on it with quill and ink. He claims that his great grandfather invented toothpaste, so you've boycotted all toothpaste out of spite.\n To the west, you can see the bathroomier.", 'North': 'Wall', 'East': 'Wall', 'South': 'Wall', 'West': 'Bathroomier'},
        'Outside': {'Item': ' ', 'description': "You walk out your front door, and approach your car.\n",'West': 'Hallway'},
        'Wall': {'description': " "},
        'Exit': {'description': " "}
    }

# Initialize collected items
collected_items = set()

while current_room != 'Exit':
    print(rooms[current_room]['description'])

    # Check for items
    if 'Item' in rooms[current_room] and rooms[current_room]['Item'] != ' ':
        item= rooms[current_room]['Item']
        if item not in collected_items:
            print(f"You see a {item} in this room. Type 'Get {item}' to get it.\n")

    valid_moves = list(rooms[current_room].keys())  # Quantifies which directions a player can go.
    move=input("Type a cardinal direction to go in, type 'inventory' to check what you've gotten so far, or choose to exit.\n").capitalize()  # Prompts player for a direction to go in.

    # Item collection functionality
    if move.startswith('Get '):
        item_to_get = move[4:]
        if 'Item' in rooms[current_room] and rooms[current_room]['Item'] == item_to_get:
            print(f"You have collected the {item_to_get}.\n")
            collected_items.add(item_to_get)
            rooms[current_room]['Item']= ' '
            collection_counter += 1
        else:
            print("You can't get that here.")
    elif move == 'Inventory':
        if collected_items:
            print("You've gotten:")
            for item in collected_items:
                print(f"- {item}")
        else:
            print("You haven't picked anything up yet.")
    else:
        # Checks move validity
        if move in valid_moves:
            new_room = rooms[current_room][move]
            # Checks if the player walked into a wall, and deals with it appropriately.
            if new_room == 'Wall':
                print("There's a wall there, but you won't let that stop you.\n"
                      "With a confident, forehead-first stance, you run full speed into the wall.\n")
                counter += 1
                if counter > 5:
                    print("That last wall you hit might have been special. \n"
                          "Either that, or 5 was your limit and you shouldn't have run head-first into a sixth one.\n")
                    current_room = 'Exit'
                else:
                    if previous_room:
                        current_room = previous_room
                        print("Not only did you not get through the wall,\n"
                              "during the short blackout, you've wandered back a room.\n")
            else:
                previous_room = current_room  # Update the previous room
                current_room = new_room
        elif move == 'Exit':
            current_room = 'Exit'
        else:   # Handles non-direction moves.
            print(move,
                  'is not a direction you can go. It might have to do with not being a direction. Try one of those.\n')

    # Checks if player can go outside, and decides their fate.
    if current_room == 'Outside':
        if collection_counter == 6:
            print("With the confidence only someone wearing clothes can feel, you decide to ignore your neighbor.\n"
                  "You approach the driver's side door of your car, pull it open, and climb inside.\n"
                  "Your neighbor never stirs, and sleeps peacefully atop your car as your drive off into the sunset.\n")
            current_room= 'Exit'
        else:
            print("As you approach your neighbor to ask him to get off of your car, he suddenly springs awake. \n"
                  "He turns toward you, but before he has the chance to say anything, your panic reaches its pinnacle, and you feel a sharp pain in your chest.\n"
                "You suffer a massive heart attack on the spot, and die. Game over.\n")
            current_room= 'Exit'

print("You've found an ending. Press enter to claim a prize based on how well you did.")
input()# Stops taking player input and ends program. Also lies to the player.

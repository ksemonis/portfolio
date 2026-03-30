# Kain Semonis
import json
import random
import hmac
import hashlib

current_room = 'Bedroom'  # Sets starting room as the Bedroom.
previous_room = None    # Gives a room to revert to if the player walks into a wall.
direction = input("Press enter to wake up.")
counter = 0  # Counts how many times the player walks into a wall.
collection_counter = 0

combat_done = False

PlayerHP = 100
PlayerATK = 1
PlayerDEF = 1
PlayerSPD = 1
PlayerLUK = 1

SECRET_KEY = b"funnykey"
SAVE_FILE = "savegame.json"


def compute_signature(data):
    encoded = json.dumps(data, sort_keys=True).encode()
    return hmac.new(SECRET_KEY, encoded, hashlib.sha256).hexdigest()


def save_game(current_room, previous_room, collected_items, counter, collection_counter):
    room_items = {room: rooms[room].get("Item") for room in rooms}

    data = {
        "current_room": current_room,
        "previous_room": previous_room,
        "collected_items": list(collected_items),
        "counter": counter,
        "collection_counter": collection_counter,
        "room_items": room_items
    }

    signature = compute_signature(data)

    full_save = {
        "data": data,
        "signature": signature
    }

    with open(SAVE_FILE, "w") as f:
        json.dump(full_save, f)
    print("Game saved\n")


def load_game():
    try:
        with open(SAVE_FILE, "r") as f:
            full_save = json.load(f)

        data = full_save["data"]
        old_signature = full_save.get("signature")

        correct_signature = compute_signature(data)

        if old_signature != compute_signature(data):
            print("We only load good saves here.\n")
            return None

        print("Loaded save\n")

        loaded_room_items = data.get("room_items", {})

        for room, item in loaded_room_items.items():
            if room in rooms:
                rooms[room]["Item"] = item

        loaded_items = set(data["collected_items"])
        recalculate_player_stats(loaded_items)

        return (
            data["current_room"],
            data["previous_room"],
            loaded_items,
            data["counter"],
            data["collection_counter"]
        )

    except FileNotFoundError:
        print("No save found\n")
        return None


# Defines the rooms and their connections, as well as adding 'walls' for the player to get amnesia from.
rooms = {
        'Bedroom': {'Item': ' ','description': "You're in a bedroom.\n You're pretty sure that it's yours, but can't be certain.\n To the east you see a hallway, and to the north you see a bathroom.",'east': 'Hallway', 'north': 'Bathroom','south': 'Wall','west': 'Wall'},
        'Hallway': {'Item': 'left shoe', 'description': "You've entered the hallway.\n The ceiling is too high on one side, and too low on the other. You can't decide if the middle is too high or too low.\n You can smell the kitchen to the north, hear the sounds of outside to the east, see another bathroom to the south, and feel the bedroom pulling you west.",'north': 'Kitchen', 'east': 'Outside', 'south': 'Bathroomier','west': 'Bedroom'},
        'Kitchen': {'Item': 'pair of socks', 'description': "You're in the kitchen.\n You wish you weren't.\n To the east you see the living room, to the south you see the hallway.",'north': 'Wall', 'east': 'Living Room', 'south': 'Hallway', 'west': 'Wall'},
        'Living Room': {'Item': 'pair of pants', 'description': "You're in the living room.\n You somehow know that the TV doesn't work, and that it might be related to not being plugged in, but you're not a high enough level wizard to be sure.\n To the west, you can still smell the kitchen.",'north': 'Wall', 'east': 'Wall','south': 'Wall', 'west': 'Kitchen'},
        'Bathroomier': {'Item': 'set of keys', 'description': "Somehow you know that this isn't just the bathroom, it's the bathroomier.\n This one actually has working plumbing, and marble tile floors.\n To the north, you see the hallway. To the east, you see the door to the late 19th century Victorian Study.", 'north': 'Hallway', 'east': 'Victorian Study', 'south': 'Wall', 'west': 'Wall'},
        'Bathroom': {'Item': 'shirt', 'description': "You're in the bathroom. It has carpeted floors.\n To the south, you can feel your bedroom pulling you.", 'north': 'Wall', 'east': 'Wall', 'south': 'Bedroom', 'west': 'Wall'},
        'Victorian Study': {'Item': 'right shoe', 'description': "You've entered Earl Stephen Chandler IV's study.\n It smells of old wood and ink. Stephen himself is sitting at his desk, bent over a piece of paper and writing on it with quill and ink. He claims that his great grandfather invented toothpaste, so you've boycotted all toothpaste out of spite.\n To the west, you can see the bathroomier.", 'north': 'wall', 'east': 'wall', 'south': 'Wall', 'west': 'Bathroomier'},
        'Outside': {'Item': ' ', 'description': "You walk out your front door, and approach your car.\n",'west': 'Hallway'},
        'wall': {'description': " "},
        'Exit': {'description': " "}
    }

enemies = {
        'Neighboar': {'HP': '100', 'ATK': '10', 'DEF': '5', 'SPD': '10', 'LUK': '10', 'HighHPDesc': '\nIt does not seem like you have done anything he cares about yet.\n', 'MidHPDesc': '\nThe Neighboar is starting to look moderately annoyed, as if he is questioning if this is worth it.\n ', 'LowHPDesc': '\nThe Neighboar seems to be having trouble standing, and looks longingly toward the small patch of grass that is your lawn.\n', 'WinDesc': '\nThe Neighboar finally collapses, and goes back to sleep in the grass next to your car.\n ', 'LoseDesc': '\nYou give a terrified screech, and feel a sharp pain in your left shoulder, something seems to be wrong.\n ', 'CommonDrop': 'Nice pants', 'RareDrop': 'Shower curtain'}
    }

item_effects = {
        "shirt": {'desc': "Your favorite and potentially only shirt.", "stats": {"ATK": 2, "DEF": 3}},
        "pair of pants": {'desc': "Your lucky pants. You would never have survived getting hit by an ambulance without them.", "stats": {"DEF": 3, "LUK": 10}},
        "left shoe": {'desc': "You loved this shoe so much that when its paired right was too worn out to wear, you kept this one.", "stats": {"ATK": 2, "SPD": 4}},
        "right shoe": {'desc': "You've had this shoe for almost a year now, it feels like it's finally coming into its own.", "stats": {"SPD": 2, "DEF": 3}},
        "pair of socks": {'desc': "You couldn't believe when the fools who sold you these wasn't charging by the sock.", "stats": {"ATK": 2, "DEF": 1}},
        "set of keys": {'desc': "You don't know what most of the keys go to, but one of them starts your car, at least.", "stats": {"ATK": 5, "SPD": 4}},
        "Nice pants": {'desc': "These pants don't have any more holes than they came out of the factory with.", "stats": {"DEF": 10}},
        "Shower curtain": {'desc': "Only the most confident can pull of such a look.", "stats": {"ATK": 10}}
}


def apply_item_stats(item_name):
    global PlayerHP, PlayerATK, PlayerDEF, PlayerSPD, PlayerLUK

    effect = item_effects.get(item_name)
    if not effect:
        return

    stats = effect.get("stats", {})

    PlayerHP += stats.get("HP", 0)
    PlayerATK += stats.get("ATK", 0)
    PlayerDEF += stats.get("DEF", 0)
    PlayerSPD += stats.get("SPD", 0)
    PlayerLUK += stats.get("LUK", 0)

    print(effect["desc"])


    # Easier way to make stats work across saves
def recalculate_player_stats(collected_items):
    global PlayerHP, PlayerATK, PlayerDEF, PlayerSPD, PlayerLUK

    PlayerHP = 10
    PlayerATK = 1
    PlayerDEF = 1
    PlayerSPD = 1
    PlayerLUK = 1

    for item in collected_items:
        apply_item_stats(item)


def combat(enemy_name):
    global PlayerHP, PlayerDEF, PlayerSPD, PlayerATK, collected_items
    global current_room

    enemy = enemies[enemy_name]

    enemy_hp = int(enemy["HP"])
    enemy_atk = int(enemy["ATK"])
    enemy_def = int(enemy["DEF"])
    enemy_spd = int(enemy["SPD"])
    enemy_luk = int(enemy["LUK"])

    max_enemy_hp = enemy_hp
    max_player_hp = PlayerHP

    print(f"\nA {enemy_name} emerges.\n")

    while PlayerHP > 0 and enemy_hp > 0:
        # Current HP descriptions and logic
        hp_ratio = enemy_hp / max_enemy_hp
        if hp_ratio > 0.75:
            print(enemy["HighHPDesc"])
        elif hp_ratio > 0.25:
            print(enemy["MidHPDesc"])
        else:
            print(enemy["LowHPDesc"])

        player_action = input("\nAttack, defend, or run.\n ").strip().lower()

        if player_action not in ["attack", "defend", "run"]:
            print("\nYou panic, and flail in the enemy's general direction, which is just as good as your normal attack, anyway.\n")
            player_action = "attack"

        if player_action == "run":
            print("\n The enemy is clearly too scary, so you decide to flee.\n"
                  "You turn to run, and promptly trip and fall, and make no effort to catch yourself.\n")
            current_room = 'Exit'
            return False

        original_def = PlayerDEF
        if player_action == "defend":
            PlayerDEF *= 3
            print("\nYou cover your face with both hands, and squeeze your eyes shut.\n")

        # Turn-order role
        player_roll = PlayerSPD * random.uniform(0.5, 1.5)
        enemy_roll = enemy_spd * random.uniform(0.5, 1.5)

        turn_order = [("player", player_roll), ("enemy", enemy_roll)]
        turn_order.sort(key=lambda x: x[1], reverse=True)

        for actor, _ in turn_order:
            if PlayerHP <= 0 or enemy_hp <= 0:
                break

            if actor == "player":
                if player_action == "attack":
                    atk_value = PlayerATK * random.uniform(0.5, 1.5)

                    # Crit check
                    if random.randint(1, 100) <= PlayerLUK:
                        atk_value *= 2
                        print("Critical hit")

                    damage = max(0, atk_value - (enemy_def / 5))
                    enemy_hp -= damage

                    print(f"You deal {int(damage)} damage. Enemy HP: {max(0, int(enemy_hp))}")
                else:
                    print(f"You do your best to defend yourself.")

            else:
                atk_value = enemy_atk * random.uniform(0.5, 1.5)

                if random.randint(1, 100) <= enemy_luk:
                    atk_value *= 2
                    print("The enemy lands a critical hit")

                damage = max(0, atk_value - (PlayerDEF / 5))
                PlayerHP -= damage

                print(f"The enemy deals {int(damage)} damage. Your HP: {max(0, int(PlayerHP))}")

        PlayerDEF = original_def

    if enemy_hp <= 0:
        print(enemy["WinDesc"])

        # Loot rolls
        if random.randint(1, 100) <= 75:
            collected_items.add(enemy["CommonDrop"])
            print(f"You obtained: {enemy['CommonDrop']}")

        if random.randint(1, 100) <= 10:
            collected_items.add(enemy["RareDrop"])
            print(f"You obtained: {enemy['RareDrop']}")

        return True

    else:
        print(enemy["LoseDesc"])
        return False


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
    move = input("Type a cardinal direction to go in, type 'inventory' to check what you've gotten so far, or choose to exit.\n\n").strip().lower()  # Prompts player for a direction to go in.

    if move == "save":
        save_game(current_room, previous_room, collected_items, counter, collection_counter)
        continue

    if move == "load":
        loaded = load_game()
        if loaded:
            current_room, previous_room, collected_items, counter, collection_counter = loaded
        continue

    # Item collection functionality
    if move.startswith('get '):
        item_to_get = move[4:]
        if 'Item' in rooms[current_room] and rooms[current_room]['Item'] == item_to_get:
            print(f"You have collected the {item_to_get}.\n")
            collected_items.add(item_to_get)
            apply_item_stats(item_to_get)
            rooms[current_room]['Item'] = ' '
            collection_counter += 1
        else:
            print("You can't get that here.")
    elif move == 'inventory':
        if collected_items:
            print("You've gotten:")
            for item in collected_items:
                print(f"- {item}")
                if item in item_effects:
                    print(f" {item_effects[item]['desc']}")
        else:
            print("You haven't picked anything up yet.")
        continue

    # Inventory sort
    elif move.startswith('sort'):
        if not collected_items:
            print("Trying to sort nothing is easy, but unproductive.\n")
            continue
        parts = move.split()

        reverse_order = False

        if len(parts) > 1:
            if parts[1] in ['desc', 'reverse']:
                reverse_order = True
            elif parts[1] not in ['asc', 'alpha']:
                print("Can't sort like that, use 'sort', 'sort asc', or 'sort desc'.\n")
                continue

        print("Your sorted inventory:")
        for item in sorted(collected_items, reverse=reverse_order):
            print(f"- {item}")
        continue

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

                if random.randint(1, 100) == 1:
                    print(f"You approach the stairs leading to the {new_room}.\n"
                    "You briefly wonder why your house came with a grand staircase between each room\n"
                    "instead of something normal, like doors.\n"
                    "While considering this, you lose your footing, and trip the rest of the way up the stairs.\n")
                    counter += 1
        elif move == 'Exit':
            current_room = 'Exit'
        else:   # Handles non-direction moves.
            print(move,
                  'is not a direction you can go. It might have to do with not being a direction. Try one of those.\n')

    # Begins neighboar fight.
    if current_room == 'Outside' and not combat_done:
        combat_done = True

        print("\nAs you step outside, your neighbour groans, and stands up off of your car."
              "You can't think of any other meaning for this action besides him wanting to fight.\n")

        survived = combat("Neighboar")

        if survived:
            print("\nYou have felled the neighboar; surely he didn't just get bored or tired and go back to sleep.\n"
                  "You have a newfound level of confidence attainable only through violence.\n"
                  "With that on your mind, you get in your car, and drive off into the sunset.\n")
            current_room = 'Exit'

        if not survived:
            print("\nYou think you also smell burnt toast for a moment.")
            current_room = 'Exit'


print("You've found an ending. Press enter to claim a prize based on how well you did.")
input()  # Stops taking player input and ends program. Also lies to the player.

import sys
sys.path.insert(0, './pypokerengine/api/')
import game
setup_config = game.setup_config
start_poker = game.start_poker
import time
from argparse import ArgumentParser

""" =========== *Remember to import your agent!!! =========== """
from randomplayer import RandomPlayer
from custom_player import CustomPlayer
from raise_player import RaisedPlayer
# from smartwarrior import SmartWarrior
""" ========================================================= """

""" Example---To run testperf.py with random warrior AI against itself. 

$ python testperf.py -n1 "Random Warrior 1" -a1 RandomPlayer -n2 "Random Warrior 2" -a2 RandomPlayer
"""

# import multiprocessing as mp

def play_games(start_game, num_games, agent_name1, agent1_class, agent_name2, agent2_class, max_round, initial_stack, smallblind_amount):
    """Worker process function to play a set of games."""
    from pypokerengine.api.game import setup_config, start_poker  # local import to ensure child processes load correctly

    agent1 = agent1_class()
    agent2 = agent2_class()

    agent1.over_limit_count = 0
    agent2.over_limit_count = 0

    config = setup_config(max_round=max_round, initial_stack=initial_stack, small_blind_amount=smallblind_amount)
    config.register_player(name=agent_name1, algorithm=agent1)
    config.register_player(name=agent_name2, algorithm=agent2)

    agent1_pot = 0
    agent2_pot = 0
    total_diff = 0

    for _ in range(num_games):
        game_result = start_poker(config, verbose=2)
        stack1 = game_result['players'][0]['stack']
        stack2 = game_result['players'][1]['stack']
        agent1_pot += stack1
        agent2_pot += stack2
        total_diff += (stack1 - stack2)

    return (agent1_pot, agent2_pot, agent1.over_limit_count, agent2.over_limit_count, total_diff)


# def testperf_mp(agent_name1, agent1_class, agent_name2, agent2_class, total_games=1000, num_processes=4):
#     max_round = 25
#     initial_stack = 1000
#     smallblind_amount = 10

#     games_per_process = total_games // num_processes
#     args = [(i * games_per_process, games_per_process,
#              agent_name1, agent1_class, agent_name2, agent2_class,
#              max_round, initial_stack, smallblind_amount) for i in range(num_processes)]

#     ctx = mp.get_context("spawn")
#     with ctx.Pool(processes=num_processes) as pool:
#         results = pool.starmap(play_games, args)
      
#     total_agent1_pot = sum(res[0] for res in results)
#     total_agent2_pot = sum(res[1] for res in results)
#     total_over_limit1 = sum(res[2] for res in results)
#     total_over_limit2 = sum(res[3] for res in results)
#     average_difference = sum(res[4] for res in results) / total_games

#     print(f"\nAfter playing {total_games} games:")
#     print(f"{agent_name1}'s final pot: {total_agent1_pot}")
#     print(f"{agent_name2}'s final pot: {total_agent2_pot}")
#     print(f"{agent_name1} failed to respond {total_over_limit1 / total_games:.2f} times per game on average.")
#     print(f"{agent_name2} failed to respond {total_over_limit2 / total_games:.2f} times per game on average.")
    
#     if total_agent1_pot > total_agent2_pot:
#         print(f"\nCongratulations! {agent_name1} has won.")
#         print(f"{agent_name1} scored {average_difference} more than {agent_name2} on average.")
#     elif total_agent1_pot < total_agent2_pot:
#         print(f"\nCongratulations! {agent_name2} has won.")
#         print(f"{agent_name2} scored {-average_difference} more than {agent_name1} on average.")
#     else:
#         print("\nIt's a draw!")

def testperf(agent_name1, agent1, agent_name2, agent2):		

	# Init to play 500 games of 1000 rounds
	num_game = 10
	max_round = 50
	initial_stack = 1000
	smallblind_amount = 10

	# Init pot of players
	agent1_pot = 0
	agent2_pot = 0

	# Setting configuration
	config = setup_config(max_round=max_round, initial_stack=initial_stack, small_blind_amount=smallblind_amount)
	
	# Register players
	# config.register_player(name=agent_name1, algorithm=RandomPlayer())
	# config.register_player(name=agent_name2, algorithm=CustomPlayer())
	config.register_player(name=agent_name1, algorithm=agent1)
	config.register_player(name=agent_name2, algorithm=agent2)
	
	average_difference = 0
	# Start playing num_game games
	for game in range(1, num_game+1):
		print("Game number: ", game)
		game_result = start_poker(config, verbose=0)
		agent1_pot = agent1_pot + game_result['players'][0]['stack']
		agent2_pot = agent2_pot + game_result['players'][1]['stack']
		average_difference += game_result['players'][0]['stack'] - game_result['players'][1]['stack']
	
	average_difference = average_difference / num_game

	print("\n After playing {} games of {} rounds, the results are: ".format(num_game, max_round))
	# print("\n Agent 1's final pot: ", agent1_pot)
	print("\n " + agent_name1 + "'s final pot: ", agent1_pot)
	print("\n " + agent_name2 + "'s final pot: ", agent2_pot)

	print("\n Agent 1 failed to respond to the action {} times per game on average.".format(agent1.over_limit_count/num_game))
	print("\n Agent 2 failed to respond to the action {} times per game on average.".format(agent2.over_limit_count/num_game))
	# print("\n ", game_result)
	# print("\n Random player's final stack: ", game_result['players'][0]['stack'])
	# print("\n " + agent_name + "'s final stack: ", game_result['players'][1]['stack'])

	if (agent1_pot<agent2_pot):
		print("\n Congratulations! " + agent_name2 + " has won.")
		print(f"Agent 2 scored {-average_difference} more than Agent 1 on average.")
	elif(agent1_pot>agent2_pot):
		print("\n Congratulations! " + agent_name1 + " has won.")
		print(f"Agent 1 scored {average_difference} more than Agent 2 on average.")
	else:
		print("\n It's a draw!") 


def parse_arguments():
    parser = ArgumentParser()
    parser.add_argument('-n1', '--agent_name1', help="Name of agent 1", default="Your agent", type=str)
    parser.add_argument('-a1', '--agent1', help="Agent 1", default=RandomPlayer())    
    parser.add_argument('-n2', '--agent_name2', help="Name of agent 2", default="Your agent", type=str)
    parser.add_argument('-a2', '--agent2', help="Agent 2", default=RandomPlayer())    
    args = parser.parse_args()
    return args.agent_name1, args.agent1, args.agent_name2, args.agent2

def make_mcts_player():
    return CustomPlayer(True)

def make_base_player():
    return CustomPlayer(False)

def make_random_player():
    return RandomPlayer()


if __name__ == '__main__':
	name1, agent1, name2, agent2 = parse_arguments()
	start = time.time()
	# testperf("MCTS Player", CustomPlayer(True), "Base Player", CustomPlayer(False))
    
	testperf("MCTS Player", CustomPlayer(True), "Base Player", RaisedPlayer())
	# testperf("MCTS Player", CustomPlayer(True), "Base Player", CustomPlayer(False))
	# testperf_mp("MCTS Player", make_mcts_player, "Base Player", make_base_player, total_games=1000, num_processes=20)	
	end = time.time()

	print("\n Time taken to play: %.4f seconds" %(end-start))
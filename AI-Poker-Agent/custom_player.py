from pypokerengine.players import BasePokerPlayer
import random as rand
import pprint
import time
import strategy_parser as sp

class CustomPlayer(BasePokerPlayer):
  def __init__(self):
    print("CustomPlayer")
    # self.strategy = sp.StrategyParser("final.iter-46620k.secs-60.avg-strategy", "states.log")
    # self.strategy = sp.StrategyParser("final.iter-34107794k.secs-43200.avg-strategy", "states.log")
    # self.strategy = sp.StrategyParser("final.iter-68351425k.secs-86400.avg-strategy", "states.log")
    self.strategy = sp.StrategyParser("final.iter-342445386k.secs-432000.avg-strategy", "states.log")

  def declare_action(self, valid_actions, hole_card, round_state):
    # time this function
    start_time = time.time()
    # get action history
    action_history = "/"
    history = round_state["action_histories"]
    preflop = history["preflop"]
    if len(preflop) > 2:
      for i in preflop[2:]:
        if i["action"] == "RAISE":
          action_history += "r"
        elif i["action"] == "CALL":
          action_history += "c"
      if "flop" in history:
        action_history += "/"
        flop = history["flop"]
        for i in flop:
          if i["action"] == "RAISE":
            action_history += "r"
          elif i["action"] == "CALL":
            action_history += "c"
        if "turn" in history:
          action_history += "/"
          flop = history["turn"]
          for i in flop:
            if i["action"] == "RAISE":
              action_history += "r"
            elif i["action"] == "CALL":
              action_history += "c"
          if "river" in history:
            action_history += "/"
            flop = history["river"]
            for i in flop:
              if i["action"] == "RAISE":
                action_history += "r"
              elif i["action"] == "CALL":
                action_history += "c"
    # get hand
    cards = hole_card[0] + hole_card[1]
    for i in round_state["community_card"]:
      cards += i
    # print("action_history: ", action_history)
    # print("cards: ", cards)
    # print("valid_actions: ", valid_actions)
    num_valid_actions = len(valid_actions)
    if action_history[-1] == 'c' or (len(action_history) > 1 and action_history[-2] == 'c' and action_history[-1] == '/'):
      num_valid_actions -= 1
    action = self.strategy.parse(action_history, cards, num_valid_actions)
    if action_history[-1] == 'c' or (len(action_history) > 1 and action_history[-2] == 'c' and action_history[-1] == '/'):
      action += 1
    if action >= len(valid_actions):
      print(valid_actions)
      print(action_history)
    result = valid_actions[action]["action"]
    # print("action: ", result)
    end_time = time.time()
    # print("time taken: ", end_time - start_time)
    return result # action returned here is sent to the poker engine

  def receive_game_start_message(self, game_info):
    pass

  def receive_round_start_message(self, round_count, hole_card, seats):
    pass

  def receive_street_start_message(self, street, round_state):
    pass

  def receive_game_update_message(self, action, round_state):
    pass

  def receive_round_result_message(self, winners, hand_info, round_state):
    pass

def setup_ai():
  return CustomPlayer()

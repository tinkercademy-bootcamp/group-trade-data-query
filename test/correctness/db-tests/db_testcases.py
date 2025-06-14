import pandas as pd
import argparse

# Parses and helps with the arguments
def parse_args():
    parser = argparse.ArgumentParser(description="Run database test cases.")
    parser.add_argument("--db", type=str, required=True, help="Database name for example small or tiny")
    parser.add_argument("--bits", type=int, default=1, help="The metrics bit flag")
    return parser.parse_args()

def create_testcases(db_name):
    db = pd.read_csv(f"data/raw/trades-{db_name}.csv")
    
    # Create a list of test cases
    
    
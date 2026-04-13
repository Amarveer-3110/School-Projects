import pandas as pd

file_path = r"3P71_dataframe.pkl"
df = pd.read_pickle(file_path)

# Target filename
target_filename = "20251124_133637_0.58.jpg"

# Filter DataFrame for the target filename
filtered_df = df[df.iloc[:, 0] == target_filename]

# Check if any matching rows were found
if filtered_df.empty:
    print(f"No entry found for {target_filename}")
else:
    with pd.option_context("display.max_rows", 50, "display.max_columns", 50):
        print(filtered_df)

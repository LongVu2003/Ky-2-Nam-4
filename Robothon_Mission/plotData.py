import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import matplotlib.colors as mpc

# create new named color 
# reference: https://stackoverflow.com/questions/76886019/create-new-named-color-in-matplotlib
def register_color(color_name, color_spec):
    mpc._colors_full_map[color_name] = color_spec
    return mpc._colors_full_map

register_color('BARE', f"#844E38")

def plotObjRGB(df):

    # Append Sum of RGB values
    df['sumRGB'] = df['objRed'] + df['objGreen'] + df['objBlue']  
    df = df.sort_values(by=['objColor','sumRGB'],ascending=[False,False])  # Sort by objColor and sumRGB
    # print(df.to_string())
    
    X = np.arange(0, len(df))  # X-axis indices

    # Extract RGB values for plotting
    Y1 = df['objRed'] 
    Y2 = df['objGreen'] 
    Y3 = df['objBlue'] 
    
    plt.figure(figsize=(12, 8))  # Set figure size

    # Set color order based on objColor
    plt.rcParams['axes.prop_cycle'] = plt.cycler(color=df["objColor"])

    # Draw vertical connections between RGB points
    Xline = [X, X, X]
    Yline = [Y1, Y2, Y3]
    plt.plot(Xline, Yline, linestyle='solid', linewidth=1.5, zorder=1)

    # Plot RGB scatter points
    plt.scatter(X, Y1, c='red', marker='o', s=10, label='objRed', zorder=2)
    plt.scatter(X, Y2, c='green', marker='o', s=10, label='objGreen', zorder=2)
    plt.scatter(X, Y3, c='blue', marker='o', s=10, label='objBlue', zorder=2)

    plt.title('objRGB')
    plt.xlim(0, len(df))
    plt.ylim(0, 250)
    plt.grid(c='gray', linewidth=0.2)  # Light gray grid for better visibility
    plt.legend()

    # Apply dark theme
    %matplotlib inline
    plt.style.use("dark_background")  

    # plt.savefig('objRGB.jpg') # Save to image
    plt.show()

# reference: https://www.w3schools.com/python/pandas/pandas_json.asp
df = pd.read_json('Lb737671_0317.json',orient="records")

plotObjRGB(df)
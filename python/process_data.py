import os
import glob
import pandas as pd
import matplotlib.pyplot as plt

# Define the directory structure
base_dir = '..'


def read_data(file_pattern, dt_column='DateTime', sep=';'):
    """
    Reads data from CSV file with file structure given by :param:`file_pattern`
    """
    # Use glob to get all CSV files in the directory structure
    csv_files = glob.glob(file_pattern, recursive=True)

    # Initialize an empty list to store dataframes
    dataframes = []

    # Read each CSV file and append to the list
    for file in csv_files:
        df = pd.read_csv(file, sep=sep)
        dataframes.append(df)

    # Concatenate all dataframes into a single dataframe
    merged_df = pd.concat(dataframes, ignore_index=True)

    # Convert DateTime to datetime and other columns to float
    # Using errors='coerce' to convert invalid datetime entries to NaT
    # Get the first non-null value to determine the format

    sample_value = merged_df[dt_column].dropna().iloc[0]
    try:
        int_value = int(sample_value)
        if len(sample_value) > 10:  # Simple heuristic: if the length is greater than 10, it's probably in milliseconds
            merged_df[dt_column] = pd.to_datetime(merged_df[dt_column], errors='coerce', unit='ms')
    except ValueError:
        merged_df[dt_column] = pd.to_datetime(merged_df[dt_column], errors='coerce')
        pass

    # Drop rows with NaT in 'DateTime' column
    merged_df = merged_df.dropna(subset=[dt_column])

    float_columns = merged_df.columns.drop(dt_column)
    merged_df[float_columns] = merged_df[float_columns].astype(float)

    # Sort the DataFrame by DateTime and drop NaT rows
    merged_df = merged_df.sort_values(by=dt_column).dropna()
    merged_df.set_index(dt_column, inplace=True)
    return merged_df


def read_pr2_data(filter=False):
    pr2_all_data = []
    # for each PR2 sensor
    for a in range(0, 2):
        pr2_pattern = os.path.join(base_dir, 'data', '**', 'pr2_sensor_' + str(a), '*.csv')
        data = read_data(pr2_pattern)
        # FILTERING
        if filter:
            for i in range(0, 6):
                selected_column = 'SoilMoistMin_' + str(i)
                # pr2_a0_data_filtered = pr2_a0_data_filtered[(pr2_a0_data_filtered[selected_column] != 0)]
                # Filter rows where a selected column is between 0 and 1
                data = data[(data[selected_column] > 0.01) & (data[selected_column] <= 1)]

        pr2_all_data.append(data)
    return pr2_all_data


def read_odyssey_data(filter=False):
    ods_data = []
    # for each Odyssey sensor
    for a in range(0, 4):
        pattern = os.path.join(base_dir, 'data_odyssey', '*U0' + str(a+1) + '*.csv')
        data = read_data(pattern, dt_column='Date/Time', sep=',')
        for i in range(5):
            selected_column = f"sensor-{i+1} %"
            new_col_name = f"odyssey_{i}"
            data[selected_column] = data[selected_column]/100
            data.rename(columns={selected_column: new_col_name}, inplace=True)
        # FILTERING
        if filter:
            for i in range(0, 6):
                selected_column = f"odyssey_{i}"
                # pr2_a0_data_filtered = pr2_a0_data_filtered[(pr2_a0_data_filtered[selected_column] != 0)]
                # Filter rows where a selected column is between 0 and 1
                data = data[(data[selected_column] > 0.01) & (data[selected_column] <= 100)]

        ods_data.append(data)
    return ods_data


# Plot some columns using matplotlib
def plot_columns(ax, df, columns):
    for column in columns:
        # dat = df[(df[column] > 0.01) & (df[column] < 1)]
        # ax.plot(dat.index, dat[column], label=column,
        #         marker='o', linestyle='-', markersize=2)
        ax.plot(df.index, df[column], label=column)

    # Add vertical lines at the start of each day
    start_of_days = df.resample('D').mean().index
    for day in start_of_days:
        ax.axvline(day, color='grey', linestyle='--', linewidth=0.5)

    ax.set_xlabel('DateTime')
    ax.set_ylabel('Values')
    ax.legend()


# Plot some columns using matplotlib
def plot_moisture_rain(ax, df, title, start_date=None, end_date=None):
    cl_name = 'SoilMoistMin'
    columns = [f"{cl_name}_{i}" for i in range(6)]

    # Select data within the datetime interval
    if start_date is not None and end_date is not None:
        interval_df = df.loc[start_date:end_date]
    elif start_date is not None:
        interval_df = df.loc[start_date:]
    elif end_date is not None:
        interval_df = df.loc[:end_date]
    else:
        interval_df = df
    filtered_df = interval_df.dropna(subset=columns)

    for column in columns:
        # ax.plot(filtered_df.index, filtered_df[column], label=column,
        #         marker='o', linestyle='-', markersize=2)
        dat = filtered_df[(filtered_df[column] > 0.01) & (filtered_df[column] < 1)]
        window_size = 5  # You can adjust the window size
        smoothed_dat = dat.rolling(window=window_size).max()
        ax.plot(smoothed_dat.index, smoothed_dat[column], label=column,
                marker='o', linestyle='-', markersize=2)

    # Add vertical lines at the start of each day
    start_of_days = interval_df.resample('D').mean().index
    for day in start_of_days:
        ax.axvline(day, color='grey', linestyle='--', linewidth=0.5)

    ax.set_xlabel('DateTime')
    ax.set_ylabel('Values')
    ax.set_title(title)
    ax.legend()

    cl_name = 'RainGauge'
    rain_df = interval_df[interval_df[cl_name]>0].dropna(subset=cl_name)
    ax2 = ax.twinx()
    ax2.plot(rain_df.index, rain_df[cl_name], 'r', label='Rain',
             marker='o', linestyle='-', markersize=2)
    ax2.set_ylabel('Rain [ml/min]')
    ax2.legend(loc='center right')


def plot_moisture_rain_comparison(ax, df, title, start_date=None, end_date=None):
    # PR2
    cl_name = 'SoilMoistMin'
    depths = [0,2]
    columns = [f"{cl_name}_{i}" for i in depths]

    # Select data within the datetime interval
    if start_date is not None and end_date is not None:
        interval_df = df.loc[start_date:end_date]
    elif start_date is not None:
        interval_df = df.loc[start_date:]
    elif end_date is not None:
        interval_df = df.loc[:end_date]
    else:
        interval_df = df
    filtered_df = interval_df.dropna(subset=columns)

    for column in columns:
        # ax.plot(filtered_df.index, filtered_df[column], label=column,
        #         marker='o', linestyle='-', markersize=2)
        dat = filtered_df[(filtered_df[column] > 0.01) & (filtered_df[column] < 1)]
        window_size = 5  # You can adjust the window size
        smoothed_dat = dat.rolling(window=window_size).max()
        ax.plot(smoothed_dat.index, smoothed_dat[column], label=column,
                marker='o', linestyle='-', markersize=2)

    # Odyssey
    columns = [f"odyssey_{i}" for i in depths]
    for column in columns:
        ax.plot(interval_df.index, interval_df[column], label=column, marker='o', linestyle='-', markersize=2)

    # Add vertical lines at the start of each day
    start_of_days = interval_df.resample('D').mean().index
    for day in start_of_days:
        ax.axvline(day, color='grey', linestyle='--', linewidth=0.5)

    ax.set_xlabel('DateTime')
    ax.set_ylabel('Values')
    ax.set_title(title)
    ax.legend()

    cl_name = 'RainGauge'
    rain_df = interval_df[interval_df[cl_name]>0].dropna(subset=cl_name)
    ax2 = ax.twinx()
    ax2.plot(rain_df.index, rain_df[cl_name], 'r', label='Rain',
             marker='o', linestyle='-', markersize=2)
    ax2.set_ylabel('Rain [ml/min]')
    ax2.legend(loc='upper right')


meteo_pattern = os.path.join(base_dir, '**', 'meteo', '*.csv')
# Set DateTime as the index
meteo_data = read_data(meteo_pattern)
# Resample the data to get samples at every 15 minutes
# meteo_data_resampled = meteo_data.resample('15min').first()
meteo_data_resampled = meteo_data.resample('15min').mean()

# Example: Plot WindSpeed and Temperature_Mean
fig, ax = plt.subplots(figsize=(10, 6))
plot_columns(ax, meteo_data_resampled, ['Humidity_Mean', 'Temperature_Mean', 'RainGauge'])
ax.set_title('Humidity, Temperature, RainGauge Over Time')
# plot_columns(ax, merged_df, ['Humidity_Mean', 'Temperature_Mean'], 'Humidity and Temperature Over Time')
fig.savefig('meteo_data.pdf', format='pdf')


# PR2 - a0 - s Oddyssey U01 u meteo stanice
# PR2 - a1 - s Oddyssey U04 pod stromy
odyssey_data = read_odyssey_data(filter=False)
odyssey_names = [f"U0{i+1}" for i in range(4)]
for i in [0,3]:
    fig, ax = plt.subplots(figsize=(10, 6))
    plot_columns(ax, odyssey_data[i], columns=[f"odyssey_{i}" for i in range(5)])
    ax.set_title(f"Odyssey{odyssey_names[i]} - Soil Moisture Mineral")
    fig.savefig(f"odyssey_data_{odyssey_names[i]}.pdf", format='pdf')

pr2_data = read_pr2_data(filter=False)
pr2_names = [f"a{i}" for i in range(2)]
merging_dates = {'start_date': '2024-07-05', 'end_date': '2024-07-15'}

# Merge the meteo and pr2 dataframes on DateTime using outer join
pr2_data_merged_a0 = pd.merge(meteo_data, pr2_data[0], how='outer', left_index=True, right_index=True, sort=True)
fig, ax = plt.subplots(figsize=(10, 6))
# plot_columns(ax, pr2_a0_data_filtered, ['SoilMoistMin_0', 'SoilMoistMin_5'], 'Soil Moisture Mineral')
# plot_columns(ax, all_data, ['SoilMoistMin_0', 'SoilMoistMin_5'], 'Soil Moisture Mineral')
plot_moisture_rain(ax, pr2_data_merged_a0, "Rain vs Soil Moisture", **merging_dates)
ax.set_title('Soil Moisture Mineral')
fig.savefig('pr2_data_a0.pdf', format='pdf')
# plt.show()

# Merge the meteo and pr2 dataframes on DateTime using outer join
pr2_data_merged_a1 = pd.merge(meteo_data, pr2_data[1], how='outer', left_index=True, right_index=True, sort=True)
fig, ax = plt.subplots(figsize=(10, 6))
# plot_columns(ax, pr2_a0_data_filtered, ['SoilMoistMin_0', 'SoilMoistMin_5'], 'Soil Moisture Mineral')
# plot_columns(ax, all_data, ['SoilMoistMin_0', 'SoilMoistMin_5'], 'Soil Moisture Mineral')
plot_moisture_rain(ax, pr2_data_merged_a1, "Rain vs Soil Moisture", **merging_dates)
# plot_moisture_rain(ax, pr2_data_merged_a1, "Rain vs Soil Moisture")
ax.set_title('Soil Moisture Mineral')
fig.savefig('pr2_data_a1.pdf', format='pdf')
# plt.show()

# fig, ax = plt.subplots(figsize=(10, 6))
# plot_columns(ax, pr2_data[0], ['SoilMoistMin_0', 'SoilMoistMin_5'])
# plot_columns(ax, pr2_data[1], ['SoilMoistMin_0', 'SoilMoistMin_5'])
# ax.set_title('Soil Moisture Mineral')
# # plot_columns(ax, all_data, ['SoilMoistMin_0', 'SoilMoistMin_5'], 'Soil Moisture Mineral')
# # plot_moisture_rain(ax, all_data, "Rain vs Soil Moisture", start_date='2024-07-05', end_date='2024-07-15')
# # fig.savefig('pr2_data.pdf', format='pdf')
# plt.show()


# PR2 - a0 - s Oddyssey U01 u meteo stanice
# PR2 - a1 - s Oddyssey U04 pod stromy
# Merge PR2 and Odyssey dataframes on DateTime using outer join
U01_data_merged = pd.merge(pr2_data_merged_a0, odyssey_data[0], how='outer', left_index=True, right_index=True, sort=True)
fig, ax = plt.subplots(figsize=(10, 6))
plot_moisture_rain_comparison(ax, U01_data_merged, "Rain vs Soil Moisture", **merging_dates)
ax.set_title('Soil Moisture Mineral')
fig.savefig('U01_data_merged.pdf', format='pdf')

U02_data_merged = pd.merge(pr2_data_merged_a1, odyssey_data[1], how='outer', left_index=True, right_index=True, sort=True)
fig, ax = plt.subplots(figsize=(10, 6))
plot_moisture_rain_comparison(ax, U02_data_merged, "Rain vs Soil Moisture", **merging_dates)
ax.set_title('Soil Moisture Mineral')
fig.savefig('U02_data_merged.pdf', format='pdf')



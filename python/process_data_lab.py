import os
import glob
import pandas as pd
import matplotlib.pyplot as plt

from process_data import read_data, read_odyssey_data


def read_pr2_data(base_dir, filter=False):
    # for each PR2 sensor
    filename_pattern = os.path.join(base_dir, '**', 'pr2_sensor', '*.csv')
    data = read_data(filename_pattern)
    # FILTERING
    if filter:
        for i in range(0, 6):
            selected_column = 'SoilMoistMin_' + str(i)
            # pr2_a0_data_filtered = pr2_a0_data_filtered[(pr2_a0_data_filtered[selected_column] != 0)]
            # Filter rows where a selected column is between 0 and 1
            data = data[(data[selected_column] > 0.01) & (data[selected_column] <= 1)]

    return data

def read_teros31_data(base_dir, filter=False):
    data = []
    sensor_names = ['A', 'B', 'C']
    for a in range(0, 3):
        filename_pattern = os.path.join(base_dir, '**', 'teros31_sensor_' + str(a), '*.csv')
        data_chunk = read_data(filename_pattern)
        # print(data_chunk)
        data_chunk.rename(columns={'Pressure': f"Pressure_{sensor_names[a]}"}, inplace=True)
        data_chunk.rename(columns={'Temperature': f"Temperature_{sensor_names[a]}"}, inplace=True)
        # print(data_chunk)
        # FILTERING
        # if filter:
            # for i in range(0, 6):
            #     selected_column = 'SoilMoistMin_' + str(i)
            #     # pr2_a0_data_filtered = pr2_a0_data_filtered[(pr2_a0_data_filtered[selected_column] != 0)]
            #     # Filter rows where a selected column is between 0 and 1
            #     data_chunk = data_chunk[(data_chunk[selected_column] > 0.01) & (data_chunk[selected_column] <= 1)]

        data.append(data_chunk)
    return data


def read_atmospheric_data(base_dir):
    filename_pattern = os.path.join(base_dir, '**', 'atmospheric', '*.csv')
    data = read_data(filename_pattern)
    return data


def read_flow_data(base_dir):
    filename_pattern = os.path.join(base_dir, '**', 'flow', '*.csv')
    data = read_data(filename_pattern)
    return data


# def read_odyssey_data(filter=False):
#     pattern = os.path.join(base_dir, 'odyssey_*.csv')
#     data = read_data(pattern, dt_column='Date/Time', sep=',')
#     for i in range(5):
#         selected_column = f"sensor-{i+1} %"
#         new_col_name = f"odyssey_{i}"
#         data[selected_column] = data[selected_column]/100
#         data.rename(columns={selected_column: new_col_name}, inplace=True)
#     # FILTERING
#     if filter:
#         for i in range(0, 6):
#             selected_column = f"odyssey_{i}"
#             # pr2_a0_data_filtered = pr2_a0_data_filtered[(pr2_a0_data_filtered[selected_column] != 0)]
#             # Filter rows where a selected column is between 0 and 1
#             data = data[(data[selected_column] > 0.01) & (data[selected_column] <= 100)]
#
#     return data


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


def select_time_interval(df, start_date=None, end_date=None):
    # Select data within the datetime interval
    if start_date is not None and end_date is not None:
        subset_df = df.loc[start_date:end_date]
    elif start_date is not None:
        subset_df = df.loc[start_date:]
    elif end_date is not None:
        subset_df = df.loc[:end_date]
    else:
        subset_df = df
    return subset_df

# Plot some columns using matplotlib
def plot_atm_data(ax, df, title):
    column = "Pressure"
    ax.plot(df.index, df[column], label=column, marker='o', linestyle='-', markersize=2, color='red')
    ax.set_ylabel('P [Pa]')

    ax2 = ax.twinx()
    column = "Temperature"
    ax2.plot(df.index, df[column], label=column, marker='o', linestyle='-', markersize=2, color='blue')
    ax2.set_ylabel('T [^oC]')
    ax2.legend(loc='center right')

    # Add vertical lines at the start of each day
    start_of_days = df.resample('D').mean().index
    for day in start_of_days:
        ax.axvline(day, color='grey', linestyle='--', linewidth=0.5)

    ax.set_xlabel('DateTime')
    ax.set_title(title)
    ax.legend()

    # Rotate the x-axis labels by 60 degrees
    for label in ax.get_xticklabels():
        label.set_rotation(30)

    df.to_csv(os.path.join(output_folder, 'atm_data_filtered.csv'), index=True)

# Plot some columns using matplotlib
def plot_pr2_data(ax, df, title):
    cl_name = 'SoilMoistMin'
    columns = [f"{cl_name}_{i}" for i in range(6)]

    filtered_df = df.dropna(subset=columns)

    for column in columns:
        ax.plot(filtered_df.index, filtered_df[column], label=column,
                marker='o', linestyle='-', markersize=2)
        # dat = filtered_df[(filtered_df[column] > 0.01) & (filtered_df[column] < 1)]
        # window_size = 5  # You can adjust the window size
        # smoothed_dat = dat.rolling(window=window_size).max()
        # ax.plot(smoothed_dat.index, smoothed_dat[column], label=column,
        #         marker='o', linestyle='-', markersize=2)

    # Add vertical lines at the start of each day
    start_of_days = df.resample('D').mean().index
    for day in start_of_days:
        ax.axvline(day, color='grey', linestyle='--', linewidth=0.5)

    ax.set_xlabel('DateTime')
    ax.set_ylabel('Values')
    ax.set_title(title)
    ax.legend()

    # Rotate the x-axis labels by 60 degrees
    for label in ax.get_xticklabels():
        label.set_rotation(30)

    filtered_df.to_csv(os.path.join(output_folder, 'pr2_data_filtered.csv'), index=True)

    # cl_name = 'Humidity'
    # hum_df = interval_df[interval_df[cl_name]>0].dropna(subset=cl_name)
    # ax2 = ax.twinx()
    # ax2.plot(hum_df.index, hum_df[cl_name], 'r', label='Humidity',
    #          marker='o', linestyle='-', markersize=2)
    # ax2.set_ylabel('Humidity [%]')
    # ax2.legend(loc='center right')


# Plot some columns using matplotlib
def plot_teros31_data(ax, df, title, diff=True):
    cl_name = 'Pressure'
    if diff:
        columns = [f"{cl_name}_{i}" for i in ['AA', 'BB' ,'CC']]
    else:
        columns = [f"{cl_name}_{i}" for i in ['A', 'B', 'C']]

    # filtered_df = interval_df.dropna(subset=columns)
    filtered_df = df[(df != 0).all(axis=1)]

    for column in columns:
        ax.plot(filtered_df.index, filtered_df[column], label=column,
                marker='o', markersize=2)
        # get last line color
        color = ax.get_lines()[-1].get_color()

        interpolated_df = filtered_df.interpolate()
        ax.plot(filtered_df.index, interpolated_df[column], label='_nolegend_', linestyle='-', color=color)
        # ax.plot(filtered_df.index, filtered_df[column], label=column,
        #         marker='o', linestyle='-', markersize=2)
        # dat = filtered_df[(filtered_df[column] > 0.01) & (filtered_df[column] < 1)]
        # window_size = 5  # You can adjust the window size
        # smoothed_dat = dat.rolling(window=window_size).max()
        # ax.plot(smoothed_dat.index, smoothed_dat[column], label=column,
        #         marker='o', linestyle='-', markersize=2)

    # Add vertical lines at the start of each day
    start_of_days = filtered_df.resample('D').mean().index
    for day in start_of_days:
        ax.axvline(day, color='grey', linestyle='--', linewidth=0.5)

    ax.set_xlabel('DateTime')
    ax.set_ylabel('Values')
    ax.set_title(title)
    ax.legend()

    # Rotate the x-axis labels by 60 degrees
    for label in ax.get_xticklabels():
        label.set_rotation(30)

    filtered_df.to_csv(os.path.join(output_folder, 'teros31_data_filtered.csv'), index=True)



if __name__ == '__main__':
    # Define the directory structure
    base_dir = '../data_lab_02'
    # Define the folder and file name
    output_folder = "lab_results_02"
    # Check if the folder exists, if not, create it
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)


    time_interval = {'start_date': '2024-11-18T00:00:00', 'end_date': '2024-11-20T17:00:00'}

    flow_data = read_flow_data(base_dir)
    # Resample the data to get samples at every 5 minutes
    # meteo_data_resampled = meteo_data.resample('5min').first()
    # flow_data_resampled = flow_data.resample('5min').mean()

    odyssey = True

    atm_data = select_time_interval(read_atmospheric_data(base_dir), **time_interval)
    pr2_data = select_time_interval(read_pr2_data(base_dir), **time_interval)
    if odyssey:
        odyssey_data = read_odyssey_data(base_dir, filter=False, ids=[5])
        odyssey_data = select_time_interval(odyssey_data[0], start_date='2024-10-01T00:00:00', end_date='2024-10-31T23:00:00')
        # odyssey_data = select_time_interval(odyssey_data[0], **time_interval)
    teros31_data = read_teros31_data(base_dir)
    for i in range(len(teros31_data)):
        teros31_data[i] = select_time_interval(teros31_data[i], **time_interval)

    data_merged = pd.merge(atm_data, pr2_data, how='outer', left_index=True, right_index=True, sort=True)

    # merging_dates = {'start_date': '2024-07-05', 'end_date': '2024-07-15'}

    fig, ax = plt.subplots(figsize=(10, 7))
    # plot_columns(ax, pr2_a0_data_filtered, ['SoilMoistMin_0', 'SoilMoistMin_5'], 'Soil Moisture Mineral')
    # plot_columns(ax, all_data, ['SoilMoistMin_0', 'SoilMoistMin_5'], 'Soil Moisture Mineral')
    # plot_pr2_data(ax, data_merged, "Rain vs Soil Moisture", **merging_dates)
    plot_atm_data(ax, atm_data, "Atmospheric data")
    fig.tight_layout()
    fig.savefig(os.path.join(output_folder, 'atm_data.pdf'), format='pdf')


    # release start: 2024-10-07T11:35:00
    fig, ax = plt.subplots(figsize=(10, 7))
    # plot_columns(ax, pr2_a0_data_filtered, ['SoilMoistMin_0', 'SoilMoistMin_5'], 'Soil Moisture Mineral')
    # plot_columns(ax, all_data, ['SoilMoistMin_0', 'SoilMoistMin_5'], 'Soil Moisture Mineral')
    # plot_pr2_data(ax, data_merged, "Rain vs Soil Moisture", **merging_dates)
    plot_pr2_data(ax, pr2_data, "Humidity vs Soil Moisture")
    ax.set_title('Soil Moisture Mineral')
    fig.tight_layout()
    fig.savefig(os.path.join(output_folder, 'pr2_data.pdf'), format='pdf')

    # plt.show()

    teros31_merged = pd.merge(teros31_data[0], teros31_data[1], how='outer', left_index=True, right_index=True, sort=True)
    teros31_merged = pd.merge(teros31_merged, teros31_data[2], how='outer', left_index=True, right_index=True, sort=True)
    teros31_merged = pd.merge_asof(teros31_merged, atm_data, on='DateTime', direction='nearest')

    teros31_merged['Pressure_AA'] = teros31_merged['Pressure_A'] - teros31_merged['Pressure']/1000
    teros31_merged['Pressure_BB'] = teros31_merged['Pressure_B'] - teros31_merged['Pressure']/1000
    teros31_merged['Pressure_CC'] = teros31_merged['Pressure_C'] - teros31_merged['Pressure']/1000
    teros31_merged.set_index('DateTime', inplace=True)

    fig, ax = plt.subplots(figsize=(10, 7))
    plot_teros31_data(ax, teros31_merged, "Teros 31", diff=False)
    ax.set_title('Total Pressure')
    fig.tight_layout()
    fig.savefig(os.path.join(output_folder, 'teros31_data_abs.pdf'), format='pdf')

    fig, ax = plt.subplots(figsize=(10, 7))
    plot_teros31_data(ax, teros31_merged, "Teros 31", diff=True)
    ax.set_title('Total Pressure')
    fig.tight_layout()
    fig.savefig(os.path.join(output_folder, 'teros31_data_diff.pdf'), format='pdf')

    if odyssey:
        fig, ax = plt.subplots(figsize=(10, 6))
        plot_columns(ax, odyssey_data, columns=[f"odyssey_{i}" for i in range(5)])
        ax.set_title("Odyssey - Soil Moisture Mineral")
        fig.savefig(os.path.join(output_folder, "odyssey_data.pdf"), format='pdf')
        odyssey_data.to_csv(os.path.join(output_folder, 'odyssey_data_filtered.csv'), index=True)

    # plt.show()

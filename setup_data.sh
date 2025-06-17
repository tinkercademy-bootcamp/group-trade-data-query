# call it as `bash setup_data.sh <team_id>` where team_id is 1,2,3 or 4
# it will automatically download, extract, clean and preprocess the data

remote_data_dir="/home/trade-data"
data_files="trades-example trades-tiny"
final_dir="./data/raw"

mkdir -p data/raw

for data_file in $data_files; do
    if [ -f "$final_dir/$data_file.csv" ]; then
        echo "$data_file.csv already exists."
        continue
    fi

    if [ -z "$1" ]; then
            echo "Usage: sh setup_data.sh <team-id>, team-id can be 1,2,3 or 4"
            exit 1
    fi

    if [ ! -f "$final_dir/$data_file.tar.gz" ]; then
        echo "Downloading $data_file.tar.gz"
        scp user@team$1.tnkr.be:"$remote_data_dir/$data_file.tar.gz" "$final_dir/$data_file.tar.gz"
    fi

    echo "Extracting $data_file.tar.gz"
    tar -xzvf "$final_dir/$data_file.tar.gz" -C "$final_dir"
done

make data
make remove_spaces
make processed_data

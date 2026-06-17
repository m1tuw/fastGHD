import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

ghd_confs={
    "bowtie":1,
    "j3":7,
    "j4":7,
    "triangle_barbell":1,
    "triangle_tadpole":1,
    "ti4":7,
    "square_tadpole":1,
    "square_barbell":1,
    "penta_barbell":1,
    "t3":7,
    "t4":7,
    "ti3":7,
}
def min_time():
    multi = ["J3", "J4", "T3", "Ti3", "T4", "Ti4"]
    for i, m in enumerate(multi):
        df1 = pd.read_csv(f"outputs_time/{m}_ghd_yk_1.csv", names=["time"])
        df2 = pd.read_csv(f"outputs_time/{m}_ghd_yk_2.csv", names=["time"])
        df3 = pd.read_csv(f"outputs_time/{m}_ghd_yk_3.csv", names=["time"])

        res = pd.merge(df1, df2, left_index=True, right_index=True, suffixes=('_1', '_2'))
        res = pd.merge(res, df3, left_index=True, right_index=True)
        res = res.min(axis=1)


""" def plot_config():
    multi = ["J3", "T3", "Ti3"] # , "T4", "Ti4", "J4"
    medianprops = dict(linestyle='-.', linewidth=2.5, color='black')

    fig = plt.figure(layout='constrained', figsize=(15,5))
    axes = fig.subplots(1, 3)
    fig.suptitle('Query Times for Decompositions')

    for i, m in enumerate(multi):
        df1 = pd.read_csv(f"outputs_time/config_{m}_ghd_yk_1.csv", names=["time"])
        df2 = pd.read_csv(f"outputs_time/config_{m}_ghd_yk_2.csv", names=["time"])
        df3 = pd.read_csv(f"outputs_time/config_{m}_ghd_yk_3.csv", names=["time"])

        res = pd.merge(df1, df2, left_index=True, right_index=True, suffixes=('_1', '_2'))
        res = pd.merge(res, df3, left_index=True, right_index=True)
        res.rename(columns={"time_1":"smallest", "time_2":"medium", "time":"largest"}, inplace=True)
        ax = axes[i%3]
        ax.set_title(m)
        ax.set_ylabel('Execution time (s)')
        #ax.set_xlabel('Which relation was left out of initial multijoin')
        bplot, props = res.boxplot(
            ax=ax, 
            column=['smallest', 'medium', 'largest'],
            patch_artist=True,
            medianprops=medianprops,
            showmeans=False,
            showfliers=False,
            return_type='both',
        )
        colors = ['firebrick', 'gold', 'cornflowerblue']

        for patch, color in zip(props['boxes'], colors):
            patch.set_facecolor(color)

    plt.savefig("outputs/triangle_ghd_configurations") """

def plot_config():
    multi = ["j3", "t3", "ti3","j4",  "t4", "ti4"]
    medianprops = dict(linestyle='-.', linewidth=2.5, color='black')

    fig = plt.figure(layout='constrained', figsize=(10, 10))
    axes = fig.subplots(3, 2)
    fig.suptitle('Times for Different Decompositions in original qdag')

    for i, m in enumerate(multi):
        res = pd.read_csv(f"outputs_time/{m}_ghd_yk_1.csv", names=["time"])
        for j in range(2, ghd_confs[m]+1):
            res = pd.merge(res, pd.read_csv(f"outputs_time/{m}_ghd_yk_{j}.csv", names=["time"]), left_index=True, right_index=True, suffixes=('', f'_{j}'))
        
        res=res.set_axis([f'config {x}' for x in range(1, ghd_confs[m]+1)], axis='columns')
        
        print(res.columns)
        
        ax = axes[i//2][i%2]
        ax.set_title(m)
        ax.set_ylabel('Execution time (s)')
        bplot, props = res.boxplot(
            ax=ax,   
            patch_artist=True,
            medianprops=medianprops,
            showmeans=False,
            showfliers=False,
            return_type='both'
        )
        ax.tick_params(axis='x', labelrotation=45)

        colors = ['firebrick', 'gold', 'cornflowerblue', 'saddlebrown', 'lightpink', 'darkseagreen', 'mediumpurple', 'darkorange', 'olive']

        for patch, color in zip(props['boxes'], colors):
            patch.set_facecolor(color)

    plt.savefig("outputs/ghd_configurations")

def plot_times():
    patterns = ["J3", "J4", "T3", "Ti3", "T4", "Ti4", "bowtie", "triangle_tadpole"]
    medianprops = dict(linestyle='-.', linewidth=2.5, color='black')
    data = {"Pattern": [], "Mean MJ": [], "Median MJ": [], "Std. dev. MJ": [], "Mean GHD": [], "Median GHD": [], "Std. dev. GHD": []}
    df = pd.DataFrame(data)
    fig = plt.figure(layout='constrained', figsize=(10, 10))
    axes = fig.subplots(4, 2)
    fig.suptitle('Times for Original Qdag Variant')
    multi = ["J3", "J4", "T3", "Ti3", "T4", "Ti4"]

    for i, p in enumerate(patterns):
        if p in multi:
            yk_res = pd.read_csv(f"outputs_time/{p}_ghd_yk_1.csv", names=["time"])
            for j in range(2, ghd_confs[p]+1):
                yk_res = pd.merge(yk_res, pd.read_csv(f"outputs_time/{p}_ghd_yk_{j}.csv", names=["time"]), left_index=True, right_index=True, suffixes=('', f'_{j}'))
            yk_res["GHD"] = yk_res.min(axis=1)
            ykp_res = pd.read_csv(f"outputs_time/{p}_ghd_yk_par_1.csv", names=["time"])
            for j in range(2, ghd_confs[p]+1):
                ykp_res = pd.merge(ykp_res, pd.read_csv(f"outputs_time/{p}_ghd_yk_par_{j}.csv", names=["time"]), left_index=True, right_index=True, suffixes=('', f'_{j}'))
            print(yk_res)
            ykp_res["GHD par"] = ykp_res.min(axis=1)
            print(ykp_res)
            """
            df_np1 = pd.read_csv(f"outputs_time/no_pruning_{p}_ghd_yk_1.csv", names=["time"])
            df_np2 = pd.read_csv(f"outputs_time/no_pruning_{p}_ghd_yk_2.csv", names=["time"])
            df_np3 = pd.read_csv(f"outputs_time/no_pruning_{p}_ghd_yk_3.csv", names=["time"])

            yk_np_res = pd.merge(df_np1, df_np2, left_index=True, right_index=True, suffixes=('_1', '_2'))
            yk_np_res = pd.merge(yk_np_res, df_np3, left_index=True, right_index=True)
            yk_np_res["GHD without pruning"] = yk_np_res.min(axis=1)
"""
            df_mj = pd.read_csv(f"outputs_time/{p}_ghd_mj.csv", names=["multijoin"])
        else:
            yk_res = pd.read_csv(f"outputs_time/{p}_yk_1.csv", names=["GHD"])
            ykp_res = pd.read_csv(f"outputs_time/{p}_yk_par_1.csv", names=["GHD par"])
            df_mj = pd.read_csv(f"outputs_time/{p}_mj.csv", names=["multijoin"])
           # yk_np_res = pd.read_csv(f"outputs_time/no_pruning_{p}_yk.csv", names=["GHD without pruning"])

        df_mj[df_mj['multijoin'] == 'timeout'] = 1800
        df_mj["multijoin"] = pd.to_numeric(df_mj["multijoin"], downcast='float')

        res = pd.merge(yk_res[["GHD"]], ykp_res[["GHD par"]], left_index=True, right_index=True)
        #res = pd.merge(res, yk_np_res, left_index=True, right_index=True)
        res = pd.merge(res, df_mj, left_index=True, right_index=True)
        ax = axes[i//2][i%2]
        ax.set_title(p)
        ax.set_ylabel('Execution time (s)')
        bplot, props = res.boxplot(
            ax=ax,
            column=["GHD", "GHD par", "multijoin"],#"GHD without pruning"],#, "multijoin"],
            patch_artist=True,
            medianprops=medianprops,
            showmeans=False,
            showfliers=False,
            return_type='both',
        )
        colors = ['firebrick', 'gold', 'cornflowerblue', 'mediumorchid']

        for patch, color in zip(props['boxes'], colors):
            patch.set_facecolor(color)
        row = {"Pattern": p, "Mean MJ": round(res.multijoin.mean(),2), "Median MJ": round(res.multijoin.median(),2), "Std. dev. MJ": round(res.multijoin.std(),2), "Mean GHD": round(res.GHD.mean(),2), "Median GHD": round(res.GHD.median(),2), "Std. dev. GHD": round(res.GHD.std(),2)}
        df.loc[len(df)] = row
    df.to_csv("outputs/og_times_stats.csv", index=False)
    handles, labels = plt.gca().get_legend_handles_labels()
    fig.legend(handles, labels, loc='upper center')
    plt.savefig("times")


def compare_times():
    patterns = ["J3", "J4", "T3", "Ti3", "T4", "Ti4", "triangle_tadpole", "bowtie"]
    multi = ["J3", "J4", "T3", "Ti3", "T4", "Ti4"]
    data = {"Pattern": [], "Mean factor mj": [], "Mean factor ghd": []}
    df = pd.DataFrame(data)
    
    for i, p in enumerate(patterns):
        if p in multi:
            df1 = pd.read_csv(f"outputs_time/{p}_ghd_yk_1.csv", names=["time"])
            df2 = pd.read_csv(f"outputs_time/{p}_ghd_yk_2.csv", names=["time"])
            df3 = pd.read_csv(f"outputs_time/{p}_ghd_yk_3.csv", names=["time"])

            yk_res = pd.merge(df1, df2, left_index=True, right_index=True, suffixes=('_1', '_2'))
            yk_res = pd.merge(yk_res, df3, left_index=True, right_index=True)
            yk_res["GHD_og"] = yk_res.min(axis=1)
            df_mj = pd.read_csv(f"outputs_time/{p}_ghd_mj.csv", names=["multijoin_og"])


            df1ha = pd.read_csv(f"/mnt/c/Users/crist/Documents/mati/ha_outputs_time/ha_{p}_ghd_yk_1.csv", names=["time"])
            df2ha = pd.read_csv(f"/mnt/c/Users/crist/Documents/mati/ha_outputs_time/ha_{p}_ghd_yk_2.csv", names=["time"])
            df3ha = pd.read_csv(f"/mnt/c/Users/crist/Documents/mati/ha_outputs_time/ha_{p}_ghd_yk_3.csv", names=["time"])
            df_mj_ha = pd.read_csv(f"/mnt/c/Users/crist/Documents/mati/ha_outputs_time/ha_{p}_ghd_mj.csv", names=["multijoin_ha"])
            ykp_res = pd.merge(df1ha, df2ha, left_index=True, right_index=True)
            ykp_res = pd.merge(ykp_res, df3ha, left_index=True, right_index=True)
            ykp_res["GHD_ha"] = ykp_res.min(axis=1)
        else:
            yk_res = pd.read_csv(f"outputs_time/{p}_yk.csv", names=["GHD_og"])
            ykp_res = pd.read_csv(f"/mnt/c/Users/crist/Documents/mati/ha_outputs_time/ha_{p}_yk.csv", names=["GHD_ha"])
            df_mj = pd.read_csv(f"outputs_time/{p}_mj.csv", names=["multijoin_og"])
            df_mj_ha = pd.read_csv(f"/mnt/c/Users/crist/Documents/mati/ha_outputs_time/ha_{p}_mj.csv", names=["multijoin_ha"])

        df_mj[df_mj['multijoin_og'] == 'timeout'] = 1800
        df_mj["multijoin_og"] = pd.to_numeric(df_mj["multijoin_og"], downcast='float')
        
        df_mj_ha[df_mj_ha['multijoin_ha'] == 'timeout'] = 1800
        df_mj_ha["multijoin_ha"] = pd.to_numeric(df_mj_ha["multijoin_ha"], downcast='float')
        res = pd.merge(yk_res[["GHD_og"]], ykp_res[["GHD_ha"]], left_index=True, right_index=True)
        res = pd.merge(res, df_mj, left_index=True, right_index=True)
        res = pd.merge(res, df_mj_ha, left_index=True, right_index=True)
        res["diff_ghd"] =  res["GHD_ha"]/res["GHD_og"]
        res["diff_mj"] =  res["multijoin_ha"]/res["multijoin_og"]
        row = {"Pattern": p, "Mean factor mj":  round(res["diff_mj"].mean(), 2), "Mean factor ghd": round(res["diff_ghd"].mean(), 2)}
        df.loc[len(df)] = row
    df.to_csv("outputs/qdag_variant_factor.csv", index=False)


def yk_times():
    patterns = ["J3", "J4", "T3", "Ti3", "T4", "Ti4", "triangle_tadpole", "bowtie"]
    multi = ["J3", "J4", "T3", "Ti3", "T4", "Ti4"]
    data = {"Pattern": [], "Mean difference": [], "Median difference": [], "Max difference": []}
    df = pd.DataFrame(data)
    data = {"GHD": [], "GHD np": [], "diff": []}
    tdf = pd.DataFrame(data)
    medianprops = dict(linestyle='-.', linewidth=2.5, color='black')
    fig = plt.figure(layout='constrained', figsize=(10, 10))
    axes = fig.subplots(4, 2)
    fig.suptitle('Times for Original Qdag Variant')
    for i, p in enumerate(patterns):
        if p in multi:
            yk_res = pd.read_csv(f"outputs_time/{p}_ghd_yk_1.csv", names=["time"])
            for j in range(2, ghd_confs[p]+1):
                yk_res = pd.merge(yk_res, pd.read_csv(f"outputs_time/{p}_ghd_yk_{j}.csv", names=["time"]), left_index=True, right_index=True, suffixes=('', f'_{j}'))
            yk_res["GHD"] = yk_res.min(axis=1)
            ykp_res = pd.read_csv(f"outputs_time/{p}_ghd_yk_par_1.csv", names=["time"])
            for j in range(2, ghd_confs[p]+1):
                ykp_res = pd.merge(ykp_res, pd.read_csv(f"outputs_time/{p}_ghd_yk_par_{j}.csv", names=["time"]), left_index=True, right_index=True, suffixes=('', f'_{j}'))
            print(yk_res)
            ykp_res["GHD par"] = ykp_res.min(axis=1)
            print(ykp_res)
        else:
            yk_res = pd.read_csv(f"outputs_time/{p}_yk_1.csv", names=["GHD"])
            yk_res = pd.read_csv(f"outputs_time/{p}_yk_par_1.csv", names=["GHD"])
            #ykp_res = pd.read_csv(f"outputs_time/no_pruning_{p}_yk.csv", names=["GHD np"])

        res = pd.merge(yk_res[["GHD"]], ykp_res[["GHD par"]], left_index=True, right_index=True)
        res["diff"] = 100*(1 - res["GHD"] / res["GHD par"])
        row = {"Pattern": p, "Mean difference": round(res["diff"].mean(),2), "Median difference": round(res["diff"].median(),2), "Max difference":round(res["diff"].max(),2)}
        df.loc[len(df)] = row
        tdf=pd.concat([tdf, res], ignore_index=True)
        ax = axes[i//2][i%2]
        ax.set_title(p)
        ax.set_ylabel('Execution time (s)')
        bplot, props = res.boxplot(
            ax=ax,
            column=["GHD", "GHD par"],
            patch_artist=True,
            medianprops=medianprops,
            showmeans=False,
            showfliers=False,
            return_type='both',
        )
        colors = ['firebrick', 'gold']

        for patch, color in zip(props['boxes'], colors):
            patch.set_facecolor(color)
        row = {"Pattern": p, "Mean difference": round(res["diff"].mean(),2), "Median difference": round(res["diff"].median(),2), "Most  hindrance":round(res["diff"].min(),2), "Most improvement":round(res["diff"].max(),2)}
        df.loc[len(df)] = row
    handles, labels = plt.gca().get_legend_handles_labels()
    fig.legend(handles, labels, loc='upper center')
    plt.savefig("outputs/og_yk_times")
    """
    df.to_csv("outputs/pruning_percentages.csv", index=False)
    fig = plt.figure(layout='constrained', figsize=(10, 10))
    fig.suptitle('Percentage change in time when using pruning')
    plt.axvline(linewidth=2, color='r')
    tdf["diff"].hist(bins=10)
    print(tdf)
    plt.savefig("outputs/pruning_diff")"""

def bpt():
    patterns = ["J3", "J4", "T3", "Ti3", "T4", "Ti4"]#, "triangle_tadpole", "bowtie"]
    multi = ["J3", "J4", "T3", "Ti3", "T4", "Ti4"]
    data = {"average": [], "median": []}
    df = pd.DataFrame(data)
    data = {"tuples": [], "qdags": [], "ratio":[]}
    res =pd.DataFrame(data)
    for i, p in enumerate(patterns):
        print(p)
        if p in multi:
            df1 = pd.read_csv(f"outputs_space/config_{p}_ghd_yk_1.csv")
        else:
            df1 = pd.read_csv(f"outputs_space/{p}_yk_1.csv")
        df1["ratio"]=df1.qdags/df1.tuples
        res = pd.concat([res, df1[["tuples", "qdags", "ratio"]]])
    row = {"average": round( res["ratio"].mean(),2), "median": round( res["ratio"].median(),2)}
    df.loc[len(df)] = row
    df.to_csv("outputs/bpt.csv", index=False)

def yk_space():
    patterns = ["J3", "T3", "Ti3", "J4", "T4", "Ti4", "triangle_tadpole", "bowtie"]
    multi = ["J3", "J4", "T3", "Ti3", "T4", "Ti4"] 
    data = {"Pattern": [], "Avg factor input": [], "Median factor input": [], "Avg factor inter": [], "Median factor inter": []}
    df = pd.DataFrame(data)
    for i, p in enumerate(patterns):
        temp = {"input":[],"inter":[]}
        df_temp = pd.DataFrame(temp)
        if p in multi:
            tdf1 = pd.read_csv(f"outputs_time/{p}_ghd_yk_1.csv", names=["time"])
            tdf2 = pd.read_csv(f"outputs_time/{p}_ghd_yk_2.csv", names=["time"])
            tdf3 = pd.read_csv(f"outputs_time/{p}_ghd_yk_3.csv", names=["time"])
            tdf1["config"]=1
            tdf2["config"]=2
            tdf3["config"]=3 
            tdf1['query'] = np.arange(0, tdf1.shape[0] )
            tdf2['query'] = np.arange(0, tdf2.shape[0] )
            tdf3['query'] = np.arange(0, tdf3.shape[0] )
            tres = pd.concat([tdf1,tdf2,tdf3])
            fastest = list(tres.sort_values('time', ascending=True).groupby('query').head(1).sort_values('query').config)
            df1 = pd.read_csv(f"outputs_space/{p}_ghd_yk_1.csv")
            df2 = pd.read_csv(f"outputs_space/{p}_ghd_yk_2.csv")
            df3 = pd.read_csv(f"outputs_space/{p}_ghd_yk_3.csv")
            df1['query'] = np.arange(0, df1.shape[0] )
            df2['query'] = np.arange(0, df2.shape[0] )
            df3['query'] = np.arange(0, df3.shape[0] )
            for i,f in enumerate(fastest):
                if f==1:
                    aux_input = df1[df1['query']==i]['qdags'].values[0]
                    aux_res = df1[df1['query']==i]['result'].values[0]
                    aux_inter = df1[df1['query']==i]['post mj'].values[0]
                elif f==2:
                    aux_input = df2[df2['query']==i]['qdags'].values[0]
                    aux_res = df2[df2['query']==i]['result'].values[0]
                    aux_inter = df2[df2['query']==i]['post mj'].values[0]
                elif f==3:
                    aux_input = df3[df3['query']==i]['qdags'].values[0]
                    aux_res = df3[df3['query']==i]['result'].values[0]
                    aux_inter = df3[df3['query']==i]['post mj'].values[0]
                row = {"input":aux_input/aux_res,"inter":[], "inter":aux_inter/aux_res}
                df_temp.loc[len(df_temp)] = row
            row = {"Pattern": p, "Avg factor input": round( df_temp["input"].mean(),2), "Median factor input": round( df_temp["input"].median(),2), "Avg factor inter": round(df_temp["inter"].mean(),2), "Median factor inter": round(df_temp["inter"].median(),2)}
            df.loc[len(df)] = row
        else:
            yk_res = pd.read_csv(f"outputs_space/{p}_yk_1.csv")
            yk_res["input"] = yk_res['qdags']/yk_res['result']
            yk_res["inter"] = yk_res['post mj']/yk_res['result']
            row = {"Pattern": p, "Avg factor input": round( yk_res["input"].mean(),2), "Median factor input": round( yk_res["input"].median(),2), "Avg factor inter": round(yk_res["inter"].mean(),2), "Median factor inter": round(yk_res["inter"].median(),2)}
            df.loc[len(df)] = row
        
    df.to_csv("outputs/avg_size.csv", index=False)

def time_comp():
    patterns = ["J3", "J4", "T3", "Ti3", "T4", "Ti4", "triangle_tadpole", "bowtie"]
    multi = ["J3", "J4", "T3", "Ti3", "T4", "Ti4"]
    data = {"Pattern": [], "Decomp beat": []}
    df = pd.DataFrame(data)
    
    for i, p in enumerate(patterns):
        if p in multi:
            df1 = pd.read_csv(f"outputs_time/{p}_ghd_yk_1.csv", names=["time"])
            df2 = pd.read_csv(f"outputs_time/{p}_ghd_yk_2.csv", names=["time"])
            df3 = pd.read_csv(f"outputs_time/{p}_ghd_yk_3.csv", names=["time"])

            yk_res = pd.merge(df1, df2, left_index=True, right_index=True, suffixes=('_1', '_2'))
            yk_res = pd.merge(yk_res, df3, left_index=True, right_index=True)
            
        else:
            yk_res = pd.read_csv(f"outputs_time/{p}_yk.csv", names=["GHD"])
            ykp_res = pd.read_csv(f"outputs_time/no_pruning_{p}_yk.csv", names=["GHD np"])

        res = pd.merge(yk_res[["GHD"]], ykp_res[["GHD np"]], left_index=True, right_index=True)
        res["diff"] = 100*(1 - res["GHD"] / res["GHD np"])
        row = {"Pattern": p, "Mean difference": round(res["diff"].mean(),2), "Median difference": round(res["diff"].median(),2), "Max difference":round(res["diff"].max(),2)}
        df.loc[len(df)] = row
        tdf=pd.concat([tdf, res], ignore_index=True)
    df.to_csv("outputs/pruning_percentages.csv", index=False)
    fig = plt.figure(layout='constrained', figsize=(10, 10))
    fig.suptitle('Percentage change in time when using pruning')
    plt.axvline(linewidth=2, color='r')
    tdf["diff"].hist(bins=10)
    print(tdf)
    plt.savefig("outputs/pruning_diff")

if __name__ == '__main__':
    plot_config()

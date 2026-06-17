
with open('/home/anouk/Documents/wikidata/property_getter/query-pattern-finder/square_tadpole.txt') as file:
    with open('/home/anouk/Documents/qdags/qdags-main/runqueries/runqueries_square_tadpole.sh', 'a') as out:
        for line in file:
            prop = line.split(',')
            out.write(f"./square_tadpole ../data/prop-direct-{prop[0]} ../data/prop-direct-{prop[1]} ../data/prop-direct-{prop[2]} ../data/prop-direct-{prop[3]} ../data/prop-direct-{prop[4]} ../data/prop-direct-{prop[5]}")

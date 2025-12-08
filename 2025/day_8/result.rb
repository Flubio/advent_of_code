# frozen_string_literal: true

class Solution
  attr_reader :input

  def initialize(input_file = 'input/input.txt')
    @input = File.read(input_file)
  end

  def part1
    vectors = lines.map { |line| line.split(',').map(&:to_i) }
    n = vectors.length
    
    # generate all pairs with their distances
    pairs = []
    (0...n).to_a.combination(2).each do |i, j|
      v1 = vectors[i]
      v2 = vectors[j]
      dist = Math.sqrt((v1[0] - v2[0])**2 + (v1[1] - v2[1])**2 + (v1[2] - v2[2])**2)
      pairs << { u: i, v: j, dist: dist }
    end

    # sort by distance and take top 1000
    top_pairs = pairs.sort_by { |p| p[:dist] }.take(1000)

    # use Union-Find to manage circuits
    uf = UnionFind.new(n)
    top_pairs.each do |pair|
      uf.union(pair[:u], pair[:v])
    end

    # get sizes of all circuits, sort descending, take top 3
    top_three_sizes = uf.component_sizes.sort.reverse.take(3)
    top_three_sizes.reduce(:*)
  end

  def part2
    vectors = lines.map { |line| line.split(',').map(&:to_i) }
    n = vectors.length
    
    pairs = []
    (0...n).to_a.combination(2).each do |i, j|
      v1 = vectors[i]
      v2 = vectors[j]
      dist = Math.sqrt((v1[0] - v2[0])**2 + (v1[1] - v2[1])**2 + (v1[2] - v2[2])**2)
      pairs << { u: i, v: j, dist: dist }
    end

    sorted_pairs = pairs.sort_by { |p| p[:dist] }

    uf = UnionFind.new(n)
    
    sorted_pairs.each do |pair|
      if uf.union(pair[:u], pair[:v])
        if uf.count == 1
          v1 = vectors[pair[:u]]
          v2 = vectors[pair[:v]]
          return v1[0] * v2[0]
        end
      end
    end
    0
  end

  private

  def lines
    @lines ||= input.split("\n")
  end

  def grid
    @grid ||= lines.map(&:chars)
  end
end

class UnionFind
  attr_reader :count

  def initialize(n)
    @parent = (0...n).to_a
    @size = Array.new(n, 1)
    @count = n
  end

  def find(i)
    return i if @parent[i] == i
    @parent[i] = find(@parent[i])
  end

  def union(i, j)
    root_i = find(i)
    root_j = find(j)
    return false if root_i == root_j

    # union by size
    if @size[root_i] < @size[root_j]
      @parent[root_i] = root_j
      @size[root_j] += @size[root_i]
    else
      @parent[root_j] = root_i
      @size[root_i] += @size[root_j]
    end
    @count -= 1
    true
  end

  def component_sizes
    # return sizes of all root nodes
    sizes = []
    @parent.each_with_index do |p, i|
      sizes << @size[i] if p == i
    end
    sizes
  end
end

# Run the solution
if __FILE__ == $PROGRAM_NAME
  solution = Solution.new
  puts "Part 1: #{solution.part1}"
  puts "Part 2: #{solution.part2}"
end
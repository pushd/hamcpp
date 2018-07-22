gem "minitest"
require "minitest/autorun"
require "hamcpp"


class TestHamcpp < Minitest::Test
  def test_hello
    #assert_equal 'hello world', Hamcpp.hello
    assert_equal 'hello, world', Hamcpp.new.hello
  end

  def test_bitset
    s = '100000101010110101010111'
    b = [s].pack('B*')
    assert_equal s, Hamcpp.new.s2bitset2s(b)
  end

  def test_null_bitset
    s = '000000000000000000000000'
    b = [s].pack('B*')
    assert_equal s, Hamcpp.new.s2bitset2s(b)
  end

  # compare two 8 bit inputs
  def test_hamming
    sa = '00000000'
    sb = '01000010'
    a = [sa].pack('B*')
    b = [sb].pack('B*')
    assert_equal 2, Hamcpp.new.hamming(a, b)
  end

  def test_hamming_exception
    sa = '00000000' * 2
    sb = '01000010'
    a = [sa].pack('B*')
    b = [sb].pack('B*')
    begin
      dist = Hamcpp.new.hamming(a, b)
      assert_equal 0, 1 # we should not get to this point
    rescue => e
      puts e.message
      assert_equal "input strings must be of equal length", e.message
    end

    # trigger one of the inputs being empty
    begin
      dist = Hamcpp.new.hamming('', '')
      assert_equal 0, 1 # we should not get to this point
    rescue => e
      puts e.message
      assert_equal "input strings must not be empty", e.message
    end
  end

  # compare two 64 bit inputs
  def test_hamming64
    sa = '00000000' * 8
    sb = '01000010' * 8
    a = [sa].pack('B*')
    b = [sb].pack('B*')
    assert_equal 8 * 2, Hamcpp.new.hamming64(a, b)
  end

  def test_hamming64_exception
    sa = '00000000' * 8
    sb = '01000010' * 7
    a = [sa].pack('B*')
    b = [sb].pack('B*')
    begin
      dist = Hamcpp.new.hamming64(a, b)
      assert_equal 0, 1 # we should not get to this point
    rescue => e
      puts e.message
      assert_equal "input strings must be of equal length", e.message
    end

    # trigger one of the inputs being empty
    begin
      dist = Hamcpp.new.hamming64('', '')
      assert_equal 0, 1 # we should not get to this point
    rescue => e
      puts e.message
      assert_equal "input strings must not be empty", e.message
    end

    # trigger one of the inputs nut having 8 multiple of bytes
    sa = '00000000' * 7
    a = [sa].pack('B*')
    # in this case both a and b violate this condition
    begin
      dist = Hamcpp.new.hamming64(a, b)
      assert_equal 0, 1 # we should not get to this point
    rescue => e
      puts e.message
      assert_equal "input strings must be multiple of 8 bytes long", e.message
    end
  end

  # compare 3 inputs provided as an array
  def test_hamming_loop
    sa = '00000000' * 2
    sb = '01000010' * 2
    sc = '01000000' * 2
    a = [sa].pack('B*')
    b = [sb].pack('B*')
    c = [sc].pack('B*')
    # expected distances:
    # a <-> b : 4  # this one will be above the chosen threshold of 2 so it should not be selected
    # a <-> c : 2
    # b <-> c : 2
    res = Hamcpp.new.hamloop([a, b, c], 2)
    assert_equal 2, res.size() # results is an array of arrays
    dist, idx_a, idx_b = res[0]
    assert_equal 2, dist
    assert_equal 0, idx_a
    assert_equal 2, idx_b

    dist, idx_a, idx_b = res[1]
    assert_equal 2, dist
    assert_equal 1, idx_a
    assert_equal 2, idx_b
  end

  # compare three 64 multiple bit inputs provided as an array
  def test_hamming_loop_64
    sa = '00000000' * 8
    sb = '01000010' * 8
    sc = '01000000' * 8
    a = [sa].pack('B*')
    b = [sb].pack('B*')
    c = [sc].pack('B*')
    # expected distances:
    # a <-> b : 16 # this one will be above the chosen threshold of 8 so it should not be selected
    # a <-> c : 8
    # b <-> c : 8
    res = Hamcpp.new.hamloop64([a, b, c], 8)
    assert_equal 2, res.size() # results is an array of arrays
    dist, idx_a, idx_b = res[0]
    assert_equal 8, dist
    assert_equal 0, idx_a
    assert_equal 2, idx_b

    dist, idx_a, idx_b = res[1]
    assert_equal 8, dist
    assert_equal 1, idx_a
    assert_equal 2, idx_b
  end


  # compare three 64 multiple bit inputs at short and long fingerprint lengths, provided as two arrays
  def test_hamming_two_stage_loop_64

    # short fingerprints
    sa = '00000000' * 8
    sb = '01000010' * 8
    sc = '01000000' * 8
    a = [sa].pack('B*')
    b = [sb].pack('B*')
    c = [sc].pack('B*')

    short_tab = [a, b, c]
    # a <-> b : 16 # this one will be above the chosen threshold of 8 so it should not be selected
    # a <-> c : 8
    # b <-> c : 8

    # short fingerprints
    sa = '00000000' * 64
    sb = '01001001' * 64
    sc = '01000000' * 64
    a = [sa].pack('B*')
    b = [sb].pack('B*')
    c = [sc].pack('B*')

    long_tab = [a, b, c]

    # expected distances:
    # a <-> b : 3 * 64  # this one will be above the chosen threshold of 64 so it should not be selected
    # a <-> c : 64
    # b <-> c : 2 * 64  # above the threshold of 64
    res = Hamcpp.new.ham2stage64(short_tab, long_tab, 8, 64)
    assert_equal 1, res.size() # results is an array of arrays
    dist, idx_a, idx_b = res[0]
    assert_equal 64, dist
    assert_equal 0, idx_a
    assert_equal 2, idx_b
  end
end

